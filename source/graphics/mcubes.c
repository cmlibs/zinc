#define MAT_CODE
#define BOOLEAN_TEST
/*******************************************************************************
FILE : mcubes.c

LAST MODIFIED : 01 November 1999

DESCRIPTION :
Modified Marching cubes isosurface generating algorithm based on David
Kenwright's work. See his PhD thesis (1993).  Two files are used to create the
look up table, MARCHG and MCUBES.  MCUBES contains a triangulated data version
for polygon drawing. These files have been generated independently using an
algorithm described in Kenwright. (arrays are shifted by -1)
???DB.  Changed load_mc_tables into a program (utilities/create_mcubes_arrays.c)
	that reads in marchg.dat and mcubes.dat and writes out static declarations
	and initializers for mctbl, ipntbl and idatbl .  Replaced load_mc_tables with
	these declarations.
==============================================================================*/
#define ACC4 0.00000001
#define ACC3 0.0000000001
/* uniqueness of vertices */
#define ACC 0.0000001
#define ACC2 0.0000001
#define AREA_ACC 0.0001
#define INTERSECTION_ACC 0.000000001
#define BLENDED_COLOURX
#define MC_EPSILON 0.000001
/* temporary vertex list size (for checking single cells) */
#define MAX_CELL_VERTICES 512
#include <stdio.h>
#include <math.h>

/*???debug */
#if defined (UNIX)
#include <sys/times.h>
#endif

#include "general/debug.h"
#include "graphics/volume_texture.h"
#include "graphics/mcubes.h"
#include "graphics/texture_line.h"
#include "graphics/complex.h"
#include "graphics/laguer.h"
#include "user_interface/message.h"

#if defined (OLD_CODE)
static int tablesloaded=0;
#endif /* defined (OLD_CODE) */

/* module data */

/* arrays defining face/edge/node connectivity */

/* binary number to represent each mc case */
static int ibin[8]={1,2,4,8,16,32,64,128};

/* edge/node relationship */
static int nodarr[12][2]={{1,5},{2,6},{4,8},{3,7},{1,2},{2,4},
	{4,3},{3,1},{5,6},{6,8},{8,7},{7,5}};

#if defined (OLD_CODE)
/* face/edge relationship */
static int fedge[6][4]={{8,4,12,1},{6,3,10,2},{1,9,2,5},{4,11,3,7},
	{5,6,7,8},{9,10,11,12}};
#endif /* defined (OLD_CODE) */

/* face/node */
static int facnod[6][4]={{1,3,7,5},{2,4,8,6},{1,5,6,2},
	{3,7,8,4},{1,2,4,3},{5,6,8,7}};

/* adjacent cell info */
static int adjcel[6][4]={{3,4,5,6},{3,4,5,6},{1,2,5,6},{1,2,5,6},
	{1,2,3,4},{1,2,3,4}};

/* table to replace triangle with more smaller ones */
static int detail_table[8][13]=
	{
		{1, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{2, 0, 3, 2, 3, 1, 2, 0, 0, 0, 0, 0, 0},
		{2, 0, 1, 4, 0, 4, 2, 0, 0, 0, 0, 0, 0},
		{3, 0, 3, 2, 3, 4, 2, 3, 1, 4, 0, 0, 0},
		{2, 0, 1, 5, 1, 2, 5, 0, 0, 0, 0, 0, 0},
		{3, 0, 3, 5, 3, 2, 5, 3, 1, 2, 0, 0, 0},
		{3, 0, 1, 5, 1, 4, 5, 4, 2, 5, 0, 0, 0},
		{4, 0, 3, 5, 3, 4, 5, 3, 1, 4, 4, 2, 5}
	};

/* triangulated marching cubes data */
static int mctbl[17][256]=
	{
		{
			0,0,0,0,0,0,5,0,0,5,0,0,0,0,0,0,0,0,3,0,
			1,0,5,0,0,5,3,0,1,0,3,0,0,3,0,0,0,3,5,0,
			2,5,0,0,2,3,0,0,0,0,0,0,1,0,5,0,2,5,0,0,
			2,2,1,0,0,1,0,1,0,0,5,0,4,5,4,4,0,0,0,0,
			0,0,3,0,0,0,5,0,4,5,4,4,0,0,3,0,6,6,6,6,
			6,6,6,6,6,6,6,6,6,6,6,6,0,0,0,0,0,0,5,0,
			4,5,4,4,2,2,0,0,0,0,2,2,4,4,5,4,0,5,0,0,
			0,0,0,0,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
			0,3,0,0,4,4,5,4,0,5,0,0,0,3,0,0,0,0,0,0,
			4,4,5,4,0,5,0,0,1,0,1,0,0,1,2,2,0,0,5,2,
			0,5,0,1,0,0,0,0,0,0,3,2,0,0,5,2,0,5,3,0,
			0,0,3,0,0,3,0,1,0,3,5,0,0,5,0,1,0,3,0,0,
			0,0,0,0,0,0,5,0,0,5,0,0,0,0,0,0
		},
		{
			0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,2,1,2,2,3,
			2,3,3,4,2,3,3,4,3,4,4,3,1,2,2,3,2,3,3,4,
			2,3,3,4,3,4,4,3,2,3,3,2,3,4,4,3,3,4,4,3,
			4,5,5,2,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,3,
			2,3,3,4,3,2,4,3,3,4,4,5,4,3,5,2,2,3,3,4,
			3,4,4,5,3,4,4,5,4,5,5,4,3,4,4,3,4,3,5,2,
			4,5,5,4,5,4,2,1,1,2,2,3,2,3,3,4,2,3,3,4,
			3,4,4,3,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,4,
			2,3,3,4,3,4,4,5,3,4,2,3,4,5,3,2,3,4,4,3,
			4,5,5,4,4,5,3,2,5,2,4,1,2,3,3,4,3,4,4,5,
			3,4,4,5,2,3,3,2,3,4,4,5,4,3,5,4,4,5,5,2,
			3,2,4,1,3,4,4,5,4,5,5,2,4,5,3,4,3,4,2,1,
			2,3,3,2,3,2,4,1,3,4,2,1,2,1,1,0
		},
		{
			0,1,2,1,4,1,2,2,3,1,2,1,3,1,2,1,1,5,1,2,
			1,4,1,2,1,3,1,2,1,3,1,2,2,1,5,1,2,2,4,1,
			2,1,3,1,2,2,3,1,1,2,1,6,4,2,4,4,3,3,1,3,
			1,2,1,3,4,1,2,4,7,1,2,1,3,1,4,4,3,1,3,1,
			1,4,2,6,1,5,2,2,3,3,1,3,1,3,1,2,2,1,4,4,
			2,2,5,1,2,1,4,4,2,2,3,1,1,2,1,4,1,2,1,6,
			3,2,1,8,1,2,1,3,3,1,2,3,3,3,2,3,6,1,2,1,
			4,1,2,1,1,3,1,3,1,3,1,3,1,5,1,2,1,4,1,2,
			2,1,3,1,4,1,4,1,2,1,5,1,2,6,4,1,1,2,1,8,
			4,7,4,3,1,2,1,7,1,2,1,4,3,1,2,1,12,1,2,1,
			4,1,7,1,6,1,5,1,4,4,2,8,1,3,2,6,1,8,1,2,
			6,5,1,2,9,1,6,1,2,5,7,1,9,1,5,1,2,5,5,1,
			1,5,4,3,2,2,1,3,1,2,1,4,1,2,1,0
		},
		{
			0,8,5,8,7,4,5,1,6,8,5,8,6,4,5,4,9,9,9,9,
			9,7,9,9,9,6,9,9,9,6,9,9,10,8,6,8,10,10,7,4,
			10,8,10,8,10,10,10,4,2,10,5,10,7,10,7,7,6,6,5,10,
			2,10,5,10,12,8,5,12,8,12,5,12,6,8,12,12,6,12,2,12,
			9,8,5,2,9,9,5,9,6,6,9,2,9,6,9,9,10,8,12,12,
			10,10,6,12,10,8,12,12,10,10,5,12,2,10,5,8,2,10,5,10,
			6,10,11,7,2,10,5,10,11,8,5,11,11,11,5,11,7,8,5,8,
			11,4,5,4,9,11,9,11,9,11,9,11,9,9,9,9,9,11,9,9,
			3,8,11,8,7,4,7,4,6,8,7,8,6,5,11,4,2,3,5,6,
			7,5,7,11,2,6,5,11,2,6,5,11,4,8,5,8,10,12,5,6,
			12,8,4,8,8,12,8,12,1,8,5,6,9,7,5,2,9,5,9,9,
			8,9,9,9,2,8,3,8,3,1,8,12,2,8,12,8,6,1,8,12,
			2,2,1,4,3,3,5,7,2,6,5,8,2,6,5,0
		},
		{
			0,5,6,2,8,5,6,6,7,5,3,2,4,6,3,2,12,8,12,12,
			12,5,12,6,12,7,12,12,12,12,12,12,9,5,9,6,9,9,8,9,
			9,5,9,7,9,9,4,9,12,5,10,8,8,12,8,6,7,7,7,12,
			12,12,12,4,11,5,6,11,11,5,6,6,7,5,11,11,8,5,11,2,
			4,5,6,8,11,7,6,6,7,7,4,7,11,11,8,3,9,5,11,11,
			9,9,9,11,9,5,11,11,9,9,8,11,10,11,4,6,8,11,6,7,
			7,11,4,3,10,11,8,11,10,5,6,10,10,10,6,10,10,5,10,2,
			8,11,10,11,12,10,12,10,12,10,12,10,12,8,12,8,12,10,12,12,
			9,5,9,6,8,5,8,6,9,5,9,7,9,1,9,9,12,5,12,3,
			8,4,8,12,6,7,7,8,12,5,12,12,10,5,6,2,3,10,6,2,
			10,5,12,2,10,10,12,2,9,5,6,2,10,5,6,9,10,9,4,10,
			1,6,8,10,3,5,4,6,7,12,12,9,6,5,9,9,8,12,9,9,
			4,3,5,6,7,5,6,6,6,7,4,7,8,5,8,0
		},
		{
			0,0,0,2,0,5,4,6,0,3,3,2,4,6,3,2,0,8,2,2,
			4,4,2,6,3,5,2,2,3,12,2,2,0,2,9,1,4,1,5,9,
			3,2,3,1,3,1,4,9,12,5,10,8,1,2,1,4,1,2,1,3,
			12,2,12,4,0,4,4,1,11,5,7,1,4,3,2,1,3,5,11,2,
			4,4,1,8,1,7,1,6,1,4,4,7,1,11,8,3,4,2,5,1,
			7,7,9,1,3,2,3,1,3,1,3,1,1,2,4,4,8,2,1,7,
			1,2,11,8,1,2,3,0,0,3,3,1,4,1,3,1,10,6,10,2,
			8,1,10,1,3,5,2,9,3,4,2,2,6,8,2,8,6,4,2,2,
			9,2,3,1,2,5,3,6,9,2,9,1,9,6,4,9,12,5,12,8,
			1,4,1,3,1,2,1,8,12,4,12,0,10,3,3,2,12,1,12,6,
			4,4,7,2,10,1,5,2,4,4,4,8,1,3,1,6,1,8,4,4,
			6,6,8,0,9,9,6,1,2,5,7,3,9,9,12,9,2,5,9,0,
			4,5,4,6,2,5,1,0,1,2,4,0,8,0,0,0
		},
		{
			0,0,0,8,0,4,7,1,0,6,5,8,6,4,5,4,0,9,5,12,
			7,5,5,9,6,9,5,12,6,6,5,12,0,10,6,6,7,4,6,4,
			6,10,9,7,6,4,10,4,2,10,5,10,2,12,5,6,2,10,7,12,
			2,12,5,10,0,12,12,8,8,12,8,6,12,6,5,8,8,12,2,12,
			9,5,9,2,11,9,9,9,9,8,9,2,11,6,9,9,12,10,6,8,
			8,5,6,11,6,10,10,8,6,12,8,11,10,11,5,6,2,11,6,10,
			2,11,1,3,10,11,10,0,0,11,11,8,7,4,11,4,7,7,5,8,
			11,11,5,11,11,9,5,12,11,7,5,9,7,9,5,9,8,10,5,12,
			3,3,9,6,3,4,11,4,6,6,7,7,6,1,9,4,2,3,5,3,
			2,5,5,12,6,7,7,11,2,11,5,0,4,4,4,8,3,10,10,1,
			10,12,12,8,8,10,12,12,9,5,1,2,10,5,9,9,10,9,9,8,
			1,9,9,0,3,2,4,6,7,12,12,7,6,2,5,8,8,12,8,0,
			2,3,5,4,7,3,6,0,6,7,5,0,2,0,0,0
		},
		{
			0,0,0,6,0,7,8,4,0,7,7,7,8,3,8,3,0,12,6,6,
			8,12,6,12,7,8,3,3,4,9,4,4,0,9,10,10,8,5,9,10,
			7,9,5,3,4,5,9,10,10,12,6,12,12,5,7,12,12,12,3,8,
			10,5,10,12,0,11,11,2,12,7,11,2,11,7,3,2,11,11,5,11,
			11,11,4,9,7,11,11,11,4,5,11,8,3,9,11,11,11,9,9,6,
			11,1,10,7,7,9,9,7,8,5,11,3,11,5,11,11,7,5,10,11,
			4,5,10,10,11,5,11,0,0,10,10,2,8,5,10,7,11,10,11,7,
			10,5,11,10,10,8,6,8,10,5,6,12,10,12,10,12,4,6,11,4,
			11,9,6,3,9,7,9,7,11,9,11,11,11,4,8,11,11,8,11,11,
			12,12,6,6,12,11,12,12,11,12,11,0,12,10,10,6,7,3,3,7,
			7,10,5,7,12,5,2,10,10,3,9,9,3,9,10,7,6,4,10,7,
			9,10,10,0,12,3,12,9,8,7,3,6,12,6,4,7,9,6,12,0,
			3,8,6,8,8,7,3,0,7,4,7,0,6,0,0,0
		},
		{
			0,0,0,0,0,0,0,4,0,0,0,2,0,6,3,0,0,0,0,6,
			0,12,4,6,0,8,3,3,4,9,4,2,0,0,0,1,0,5,9,10,
			0,3,3,1,4,5,4,10,0,5,1,0,12,5,7,12,12,2,1,3,
			3,5,10,0,0,0,0,2,0,7,11,6,0,4,3,2,11,5,11,2,
			0,11,4,8,1,0,1,6,4,4,2,8,1,9,8,0,0,4,9,1,
			11,7,7,1,4,3,3,1,3,5,11,1,1,5,11,11,7,5,1,0,
			4,5,10,8,1,5,0,0,0,0,0,2,0,5,4,1,0,10,11,2,
			8,5,11,1,0,8,3,9,4,4,3,2,10,6,10,2,6,4,2,2,
			0,9,6,1,9,2,3,1,11,9,0,1,11,6,8,0,11,8,11,8,
			12,12,1,6,12,2,12,0,11,0,11,0,0,10,10,3,12,1,12,7,
			7,4,5,2,0,5,2,0,4,3,4,8,1,3,1,7,1,4,4,0,
			6,0,8,0,12,9,6,9,2,7,3,0,12,9,4,9,9,6,0,0,
			0,8,4,0,2,0,1,0,1,2,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,0,7,0,0,0,7,0,5,8,0,0,0,0,12,
			0,5,7,12,0,9,5,12,6,6,5,4,0,0,0,10,0,4,6,4,
			0,6,5,3,6,4,9,4,0,12,10,0,2,12,5,6,2,12,3,8,
			6,12,5,0,0,0,0,8,0,12,8,12,0,12,5,8,8,11,5,11,
			0,5,9,9,7,0,11,11,9,5,5,2,3,6,11,0,0,12,6,6,
			8,1,8,7,12,6,9,7,8,12,8,3,11,11,5,6,2,11,10,0,
			2,11,1,10,11,11,0,0,0,0,0,8,0,4,7,7,0,7,5,7,
			10,11,5,10,0,9,11,8,7,5,11,12,7,7,5,8,4,6,11,4,
			0,3,9,3,3,3,9,6,6,6,0,11,6,4,9,0,2,3,5,11,
			2,5,6,12,6,11,7,0,2,0,5,0,0,4,4,4,7,3,3,1,
			10,10,12,7,0,10,12,0,10,5,9,9,3,9,10,9,6,9,10,0,
			9,0,10,0,3,3,12,6,8,12,12,0,6,6,5,7,8,12,0,0,
			0,3,6,0,8,0,3,0,7,4,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,0,6,0,0,0,3,0,1,4,0,0,0,0,8,
			0,9,8,7,0,12,7,7,8,5,8,3,0,0,0,9,0,7,10,7,
			0,7,7,9,8,6,8,3,0,8,12,0,10,4,6,10,10,5,12,7,
			4,4,3,0,0,0,0,6,0,11,12,7,0,11,7,7,12,6,12,3,
			0,9,11,4,8,0,7,7,11,11,3,9,8,5,3,0,0,11,10,9,
			12,12,11,6,11,7,5,3,11,6,12,9,4,4,6,10,10,7,11,0,
			11,4,5,4,8,3,0,0,0,0,0,6,0,7,8,6,0,11,7,11,
			6,10,8,2,0,12,10,2,8,12,10,6,11,10,11,7,11,5,10,11,
			0,11,5,9,11,9,6,9,7,11,0,9,4,11,5,0,3,11,6,12,
			11,2,12,7,7,12,11,0,6,0,8,0,0,12,12,10,8,5,7,12,
			6,7,2,10,0,6,10,0,3,10,10,4,8,10,3,10,7,10,7,0,
			10,0,5,0,4,12,5,3,9,3,9,0,7,12,7,12,12,9,0,0,
			0,4,3,0,1,0,8,0,4,5,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,7,0,0,0,7,0,3,2,0,0,0,0,0,0,0,0,7,
			0,0,0,9,0,6,8,0,0,0,0,0,0,5,1,0,0,5,12,0,
			4,5,3,0,0,0,0,0,0,0,0,7,0,0,0,2,0,6,12,0,
			0,0,0,4,0,0,1,0,0,11,3,8,8,0,8,0,0,0,0,9,
			0,7,11,1,0,4,3,1,11,6,10,9,0,5,11,0,7,0,1,0,
			11,5,10,4,8,5,0,0,0,0,0,0,0,0,0,1,0,0,0,2,
			0,5,11,0,0,0,0,2,0,12,4,6,0,10,11,2,6,4,11,2,
			0,0,0,9,0,9,6,9,0,11,0,0,4,6,0,0,0,8,11,0,
			11,12,12,7,12,2,0,0,11,0,11,0,0,0,0,10,0,5,12,7,
			0,7,2,10,0,0,0,0,0,10,4,4,8,0,1,7,1,4,7,0,
			0,0,5,0,0,12,5,9,9,3,3,0,12,12,0,12,0,6,0,0,
			0,0,0,0,0,0,8,0,0,5,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,12,0,0,0,12,0,12,4,0,0,0,0,0,0,0,0,10,
			0,0,0,3,0,4,9,0,0,0,0,0,0,4,7,0,0,12,3,0,
			6,4,5,0,0,0,0,0,0,0,0,12,0,0,0,7,0,11,5,0,
			0,0,0,9,0,0,7,0,0,5,5,9,3,0,3,0,0,0,0,6,
			0,12,8,6,0,12,5,3,8,12,9,3,0,4,6,0,10,0,11,0,
			2,4,5,10,11,3,0,0,0,0,0,0,0,0,0,6,0,0,0,11,
			0,10,8,0,0,0,0,8,0,5,7,12,0,7,5,7,11,5,5,11,
			0,0,0,3,0,3,9,6,0,6,0,0,6,11,0,0,0,11,6,0,
			2,2,6,12,7,12,0,0,6,0,8,0,0,0,0,4,0,3,7,12,
			0,10,12,7,0,0,0,0,0,5,10,9,3,0,3,10,7,10,10,0,
			0,0,10,0,0,3,12,3,8,12,9,0,7,6,0,7,0,9,0,0,
			0,0,0,0,0,0,3,0,0,4,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,4,0,0,0,8,0,4,3,0,0,0,0,0,0,0,0,6,
			0,0,0,10,0,3,5,0,0,0,0,0,0,7,12,0,0,8,10,0,
			8,6,4,0,0,0,0,0,0,0,0,11,0,0,0,3,0,3,8,0,
			0,0,0,11,0,0,8,0,0,9,7,4,6,0,2,0,0,0,0,10,
			0,11,12,10,0,11,7,9,12,11,5,10,0,8,10,0,11,0,7,0,
			10,8,3,11,3,6,0,0,0,0,0,0,0,0,0,2,0,0,0,10,
			0,6,4,0,0,0,0,6,0,9,8,4,0,11,7,10,10,9,4,10,
			0,0,0,11,0,11,5,11,0,7,0,0,8,9,0,0,0,12,3,0,
			3,11,3,4,11,8,0,0,4,0,4,0,0,0,0,12,0,7,8,3,
			0,6,10,4,0,0,0,0,0,9,3,10,7,0,8,3,4,7,2,0,
			0,0,2,0,0,4,9,12,12,2,6,0,4,7,0,4,0,2,0,0,
			0,0,0,0,0,0,7,0,0,8,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,6,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,4,0,0,8,0,0,0,0,0,
			0,0,0,1,0,0,0,9,0,6,10,0,0,0,0,0,0,0,1,0,
			0,3,3,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,10,0,4,4,0,
			0,0,0,0,0,0,0,11,0,0,0,0,0,6,0,0,0,0,0,0,
			0,11,12,0,0,2,0,0,4,0,0,0,0,0,0,0,0,0,0,3,
			0,0,0,10,0,0,0,0,0,0,0,4,0,0,8,0,0,7,7,0,
			0,0,0,0,0,0,0,12,0,2,6,0,0,12,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,4,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,9,0,0,2,0,0,0,0,0,
			0,0,0,10,0,0,0,3,0,11,5,0,0,0,0,0,0,0,7,0,
			0,6,5,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,7,0,9,5,0,
			0,0,0,0,0,0,0,6,0,0,0,0,0,9,0,0,0,0,0,0,
			0,2,3,0,0,8,0,0,6,0,0,0,0,0,0,0,0,0,0,12,
			0,0,0,4,0,0,0,0,0,0,0,10,0,0,3,0,0,10,2,0,
			0,0,0,0,0,0,0,3,0,12,9,0,0,7,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,3,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,11,0,0,5,0,0,0,0,0,
			0,0,0,9,0,0,0,10,0,3,3,0,0,0,0,0,0,0,8,0,
			0,7,7,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,11,0,12,8,0,
			0,0,0,0,0,0,0,3,0,0,0,0,0,2,0,0,0,0,0,0,
			0,3,11,0,0,5,0,0,8,0,0,0,0,0,0,0,0,0,0,10,
			0,0,0,12,0,0,0,0,0,0,0,3,0,0,7,0,0,6,5,0,
			0,0,0,0,0,0,0,4,0,9,5,0,0,4,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		}
	};

#if defined (OLD_CODE)
/* index pointer (into data table) array */
static int ipntbl[5][256]=
	{
		{
			0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
		},
		{
			0,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,
			2,1,3,1,2,2,2,1,2,1,2,1,1,2,1,1,2,2,2,1,
			2,3,1,1,2,2,1,1,1,1,1,1,2,1,2,1,2,2,1,1,
			2,1,1,1,1,2,2,2,1,1,2,1,2,3,2,2,1,1,1,1,
			1,1,2,1,1,1,2,1,2,2,2,1,1,1,1,1,2,3,2,2,
			2,2,2,1,3,4,2,2,2,2,1,1,1,1,1,1,1,1,1,1,
			2,2,1,1,1,1,2,1,1,2,2,2,2,2,3,2,1,2,1,1,
			1,1,1,1,2,2,3,2,3,2,4,2,2,2,2,1,2,1,2,1,
			1,2,1,1,2,2,2,1,1,2,1,1,1,1,1,1,1,1,1,1,
			2,1,2,1,1,1,1,1,1,2,1,1,1,2,2,2,1,1,2,1,
			1,2,1,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1,1,2,
			1,1,1,1,1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
		},
		{
			0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,
			1,0,1,0,0,1,1,0,1,0,1,0,0,1,0,0,0,1,1,0,
			1,1,0,0,1,1,0,0,0,0,0,0,1,0,1,0,1,1,0,0,
			1,1,1,0,0,1,0,1,0,0,1,0,1,1,1,1,0,0,0,0,
			0,0,1,0,0,0,1,0,1,1,1,1,0,0,1,0,1,1,1,1,
			1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,
			1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,1,0,0,
			0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			0,1,0,0,1,1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,
			1,1,1,1,0,1,0,0,1,0,1,0,0,1,1,1,0,0,1,1,
			0,1,0,1,0,0,0,0,0,0,1,1,0,0,1,1,0,1,1,0,
			0,0,1,0,0,1,0,1,0,1,1,0,0,1,0,1,0,1,0,0,
			0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,
			1,0,2,0,0,1,1,0,1,0,2,0,0,1,0,0,0,1,1,0,
			1,2,0,0,1,2,0,0,0,0,0,0,1,0,2,0,1,2,0,0,
			2,2,2,0,0,1,0,1,0,0,1,0,1,2,1,2,0,0,0,0,
			0,0,1,0,0,0,2,0,1,2,2,2,0,0,2,0,1,2,1,2,
			1,2,2,2,2,4,2,3,2,3,2,2,0,0,0,0,0,0,2,0,
			2,3,2,2,2,2,0,0,0,0,1,1,1,1,2,2,0,1,0,0,
			0,0,0,0,1,1,2,2,2,2,4,3,1,2,2,2,2,2,3,2,
			0,1,0,0,1,2,2,2,0,2,0,0,0,2,0,0,0,0,0,0,
			2,2,3,2,0,2,0,0,2,0,2,0,0,1,1,2,0,0,2,2,
			0,2,0,2,0,0,0,0,0,0,2,2,0,0,3,2,0,2,2,0,
			0,0,2,0,0,2,0,2,0,2,2,0,0,3,0,2,0,2,0,0,
			0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0
		},
		{
			0,0,0,0,0,0,5,0,0,5,0,0,0,0,0,0,0,0,3,0,
			1,0,5,0,0,5,3,0,1,0,3,0,0,3,0,0,0,3,5,0,
			2,5,0,0,2,3,0,0,0,0,0,0,1,0,5,0,2,5,0,0,
			2,2,1,0,0,1,0,1,0,0,5,0,4,5,4,4,0,0,0,0,
			0,0,3,0,0,0,5,0,4,5,4,4,0,0,3,0,6,6,6,6,
			6,6,6,6,6,6,6,6,6,6,6,6,0,0,0,0,0,0,5,0,
			4,5,4,4,2,2,0,0,0,0,2,2,4,4,5,4,0,5,0,0,
			0,0,0,0,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
			0,3,0,0,4,4,5,4,0,5,0,0,0,3,0,0,0,0,0,0,
			4,4,5,4,0,5,0,0,1,0,1,0,0,1,2,2,0,0,5,2,
			0,5,0,1,0,0,0,0,0,0,3,2,0,0,5,2,0,5,3,0,
			0,0,3,0,0,3,0,1,0,3,5,0,0,5,0,1,0,3,0,0,
			0,0,0,0,0,0,5,0,0,5,0,0,0,0,0,0
		},
	};
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
/* data table */
static int idatbl[3001]=
	{
			3,1,5,8,8,4,8,7,5,4,8,7,6,4,11,12,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0
	};
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
/* triangulated marching cubes data */
static int mctbl[17][256];

/* index pointer (into data table) array */
static int ipntbl[5][256];

/* data table */
static int idatbl[3001];
#endif /* defined (OLD_CODE) */

/* current cell node coords */
double xc[8],yc[8],zc[8];

/* scalar values at nodes */
double fc[MAX_SCALAR_FIELDS][8];

/* material indices at nodes */
double mc[8];

#if defined (OLD_CODE)
/* active clipping vertices */
static int clip_vertices[8][2]=
	{{0,0},{4,6},{4,5},{5,6},{5,6},{4,5},{4,6},{0,0}};
#endif /* defined (OLD_CODE) */

/* triangle edge relationships */
int triangle_edge_list[4][2]={ {0,1}, {1,2}, {2,0},{0,1} };

/* retriangulation list for complete cut off */
int retriangulation_list[8][7]={ {1,1,2,3,4,5,6},{2,4,2,6,2,3,6},
	{2,1,4,5,1,5,3},{1,5,3,6,2,3,1},{2,1,2,5,1,5,6},{1,4,2,5,1,2,3},
	{1,1,4,6,1,2,3},{1,1,2,3,4,5,6} };

static int n_clipped_triangles;
static struct Triangle *clipped_triangle_list[MAX_INTERSECTION];
struct Cell_fn cell_fn;

double cube[8][3]=
	{{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}};

int debug_flag;
int debug_i=0,debug_j=0,debug_k=0;

/*
Module functions
----------------
*/
static int shared_edge(double v1[3],double v2[3],double t2[3][3])
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Checks for shared edge in MC triangulation
==============================================================================*/
{
	int m,n,return_code;

	ENTER(shared_edge);
	return_code=0;
	for (n=0;n<3;n++)
	{
		m=n+1;
		if (3==m)
		{
			m=0;
		}
		if (((v1[0]==t2[n][0])&&(v1[1]==t2[n][1])&&(v1[2]==t2[n][2])&&
			(v2[0]==t2[m][0])&&(v2[1]==t2[m][1])&&(v2[2]==t2[m][2]))||
			((v1[0]==t2[m][0])&&(v1[1]==t2[m][1])&&(v1[2]==t2[m][2])&&
			(v2[0]==t2[n][0])&&(v2[1]==t2[n][1])&&(v2[2]==t2[n][2])))
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* shared_edge */

static int internal_boundary(double v1[3],double v2[3],double xc[8],
	double yc[8],double zc[8],int i,int j,int k,int *detail_map,int mcnx,int mcny,
	int mcnz, int res)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Checks for shared boundary edge in mc triangulation
==============================================================================*/
{
	int return_code;

	ENTER(internal_boundary);
  USE_PARAMETER(mcnz);
#ifdef OLD_CODE
	if (((v1[0]==xc[0])&&(v2[0]==xc[0])&&(detail_map[(i-1)+j*mcnx+k*mcnx*mcny]))||
		((v1[0]==xc[7])&&(v2[0]==xc[7])&&(detail_map[(i+1)+j*mcnx+k*mcnx*mcny]))||
		((v1[1]==yc[0])&&(v2[1]==yc[0])&&(detail_map[i+(j-1)*mcnx+k*mcnx*mcny]))||
		((v1[1]==yc[7])&&(v2[1]==yc[7])&&(detail_map[i+(j+1)*mcnx+k*mcnx*mcny]))||
		((v1[2]==zc[0])&&(v2[2]==zc[0])&&(detail_map[i+j*mcnx+(k-1)*mcnx*mcny]))||
		((v1[2]==zc[7])&&(v2[2]==zc[7])&&(detail_map[i+j*mcnx+(k+1)*mcnx*mcny])))
	{
		return_code=1;
	}
	else
	{
		return_code=0;
	}
#endif
	if (((v1[0]==xc[0])&&(v2[0]==xc[0])&&(res+1 <= detail_map[(i-1)+j*mcnx+k*mcnx*mcny]))||
		((v1[0]==xc[7])&&(v2[0]==xc[7])&&(res+1 <= detail_map[(i+1)+j*mcnx+k*mcnx*mcny]))||
		((v1[1]==yc[0])&&(v2[1]==yc[0])&&(res+1 <= detail_map[i+(j-1)*mcnx+k*mcnx*mcny]))||
		((v1[1]==yc[7])&&(v2[1]==yc[7])&&(res+1 <= detail_map[i+(j+1)*mcnx+k*mcnx*mcny]))||
		((v1[2]==zc[0])&&(v2[2]==zc[0])&&(res+1 <= detail_map[i+j*mcnx+(k-1)*mcnx*mcny]))||
		((v1[2]==zc[7])&&(v2[2]==zc[7])&&(res+1 <= detail_map[i+j*mcnx+(k+1)*mcnx*mcny])))
	{
		return_code=1;
	}
	else
	{
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* internal_boundary */

static int compile_mc_vertex_triangle_lists(
	struct MC_iso_surface *mc_iso_surface,int closed_surface)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
==============================================================================*/
{
	int triangle_index,vertex_index;
	int x_min,x_max,y_min,y_max,z_min,z_max;
	int ii,jj,kk,mcnx,mcny;
	int i,j,k,m,n,index,n_triangles,n_cv,found_in_cell,prior_cell;
	struct MC_cell *mc_cell;
	struct MC_vertex *v;
	struct MC_vertex *cell_vertices[MAX_CELL_VERTICES];
	struct MC_triangle *triangle, **triangle_list;

	ENTER(compile_mc_vertex_triangle_lists);
#if defined (OLD_CODE)
	x_min=mc_iso_surface->active_block[0];
	x_max=mc_iso_surface->active_block[1];
	y_min=mc_iso_surface->active_block[2];
	y_max=mc_iso_surface->active_block[3];
	z_min=mc_iso_surface->active_block[4];
	z_max=mc_iso_surface->active_block[5];
#endif /* defined (OLD_CODE) */
	x_min=1;
	x_max=mc_iso_surface->dimension[0];
	y_min=1;
	y_max=mc_iso_surface->dimension[1];
	z_min=1;
	z_max=mc_iso_surface->dimension[2];
	if (closed_surface)
	{
		if (1==x_min)
		{
			x_min=0;
		}
		if (1==y_min)
		{
			y_min=0;
		}
		if (1==z_min)
		{
			z_min=0;
		}
		if (mc_iso_surface->dimension[0]==x_max)
		{
			x_max += 1;
		}
		if (mc_iso_surface->dimension[1]==y_max)
		{
			y_max += 1;
		}
		if (mc_iso_surface->dimension[2]==z_max)
		{
			z_max += 1;
		}
	}
/*???debug */
/*printf("Compiling triangle and vertex lists....\n");*/
	if (mc_iso_surface->compiled_vertex_list)
	{
		DEALLOCATE(mc_iso_surface->compiled_vertex_list);
	}
	if (mc_iso_surface->compiled_triangle_list)
	{
		DEALLOCATE(mc_iso_surface->compiled_triangle_list);
	}
	triangle_index=vertex_index=0;
	if (ALLOCATE(mc_iso_surface->compiled_vertex_list,struct MC_vertex *,
		mc_iso_surface->n_vertices)&&ALLOCATE(mc_iso_surface->
		compiled_triangle_list,struct MC_triangle *,mc_iso_surface->n_triangles))
	{
		mcnx=mc_iso_surface->dimension[0]+2;
		mcny=mc_iso_surface->dimension[1]+2;
		/*mcnz=mc_iso_surface->dimension[2]+2;*/
		for (index=0;index<mc_iso_surface->n_scalar_fields+6;index++)
		{
			for (ii=x_min;ii<=x_max;ii++)
			{
				for (jj=y_min;jj<=y_max;jj++)
				{
					for (kk=z_min;kk<=z_max;kk++)
					{
						/* step through non_null cells, add vertex lists of any new triangle
							vertices in cell or pointing to forward cells */
						i=ii+jj*mcnx+kk*mcnx*mcny;
						if (mc_iso_surface->mc_cells[i] !=NULL)
						{
							if (mc_iso_surface->mc_cells[i]->triangle_list[index] !=NULL)
							{
								mc_cell=mc_iso_surface->mc_cells[i];
								triangle_list=mc_cell->triangle_list[index];
								n_triangles=mc_cell->n_triangles[index];
								n_cv=0;
								for (k=0;k<MAX_CELL_VERTICES;k++)
								{
									cell_vertices[k]=NULL;
								}
								for (j=0;j<n_triangles;j++)
								{
									triangle=triangle_list[j];
									mc_iso_surface->compiled_triangle_list[triangle_index]=
										triangle;
									triangle->triangle_index=triangle_index;
									triangle_index++;
									/* step through vertices */
									for (n=0;n<3;n++)
									{
										v=triangle->vertices[n];
										found_in_cell=0;
										prior_cell=0;
										for (k=0;k<n_cv;k++)
										{
											if ((v==cell_vertices[k])&&!found_in_cell)
											{
												found_in_cell=1;
												if ((v->vertex_index>=0)&&
													(v->vertex_index<mc_iso_surface->n_vertices))
												{
													triangle->vertex_index[n]=v->vertex_index;
												}
/*???debug */
else
{
	printf("ERROR: (1)compile vt list: v->vertex_index=%d\n", v->vertex_index);
}
											}
										}
										if (!found_in_cell)
										{
											/* check prior cells through triangles */
											for (m=0;m<v->n_triangle_ptrs;m++)
											{
												/* if the pointed to cell is less than the current it
													must have been visited already.  Note that the sense
													of increasing index has been reversed as we step
													fastest in x */
												if ((v->triangle_ptrs[m]->cell_ptr->index[2]+
													v->triangle_ptrs[m]->cell_ptr->index[1]*mcnx+
													v->triangle_ptrs[m]->cell_ptr->index[0]*mcnx*mcny<
													mc_cell->index[2]+mc_cell->index[1]*mcnx+
													mc_cell->index[0]*mcnx*mcny)&&!prior_cell)
												{
													prior_cell=1;
													/* check vertex index sensible */
													if ((v->vertex_index>=0)&&
														(v->vertex_index<mc_iso_surface->n_vertices))
													{
														triangle->vertex_index[n]=v->vertex_index;
													}
/*???debug */
else
{
	printf("ERROR: (2) compile vt list: v->vertex_index=%d\n", v->vertex_index);
	printf("v (%p)=%f %f %f\n",v,  v->coord[0], v->coord[1], v->coord[2]);
	printf("compare cell (%d %d %d)=%d to prev cell (%d %d %d)=%d\n",
		mc_cell->index[0],mc_cell->index[1], mc_cell->index[2],
		mc_cell->index[0]+mc_cell->index[1]*mcnx+mc_cell->index[2]*mcnx*mcny,
		v->triangle_ptrs[m]->cell_ptr->index[0],
		v->triangle_ptrs[m]->cell_ptr->index[1],
		v->triangle_ptrs[m]->cell_ptr->index[2],
		v->triangle_ptrs[m]->cell_ptr->index[0]+
		v->triangle_ptrs[m]->cell_ptr->index[1]*mcnx+
		v->triangle_ptrs[m]->cell_ptr->index[2]*mcnx*mcny);
}
												}
											}
										}
										if (!found_in_cell && !prior_cell)
										{
											/* store vertex */
											cell_vertices[n_cv]=v;
											if (n_cv < MAX_CELL_VERTICES)
											{
												n_cv++;
											}
											else
											{
												display_message(ERROR_MESSAGE,
							"compile_mc_vertex_triangle_lists.  Increase MAX_CELL_VERTICES");
/*???debug */
printf("MAX CELL VERTICES REACHED\n");
											}
											mc_iso_surface->compiled_vertex_list[vertex_index]=v;
/*???debug */
if (vertex_index > mc_iso_surface->n_vertices || vertex_index < 0)
{
	int mm;

	printf("VERTEX %d INDEX ERROR: mc_cell (%d %d %d) <%p>\n",vertex_index,
		ii, jj, kk, mc_iso_surface->mc_cells[i]);
	printf("v (%d %d %d) <%p>\n", ii, jj, kk, mc_iso_surface->mc_cells[i]);
	printf("v->n_triangle_ptrs=%d :", v->n_triangle_ptrs);
	for (mm=0;mm< v->n_triangle_ptrs;mm++)
	{
		printf(" %p (%d) : ", v->triangle_ptrs[mm],
			v->triangle_ptrs[mm]->triangle_index);
	}
	printf("\n");
	for (mm=0;mm< v->n_triangle_ptrs;mm++)
	{
		printf("(%d)  %d %d %d : ", v->triangle_ptrs[mm]->triangle_index,
			v->triangle_ptrs[mm]->cell_ptr->index[0],
		v->triangle_ptrs[mm]->cell_ptr->index[1],
		v->triangle_ptrs[mm]->cell_ptr->index[2]);
	}
/*???DB.  Not allowed.  Have to handle errors better */
/*	exit(0);*/
}
											v->vertex_index=vertex_index;
											triangle->vertex_index[n]=v->vertex_index;
											vertex_index++;

										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compile_mc_vertex_triangle_lists.  Alloc failed");
	}
/*???debug */
/*printf("-------Compiled list of %d vertices and %d triangles---------\n",
	vertex_index,triangle_index);*/
	LEAVE;

	return (triangle_index);
} /* compile_mc_vertex_triangle_lists */

static struct MC_vertex *check_mc_vertex(struct MC_cell *mc_cell,float v[3],
	int a)
/*******************************************************************************
LAST MODIFIED : 26 March 1997

DESCRIPTION :
Compare vertices. If found return mc_vertex else return NULL.
==============================================================================*/
{
	int i, j, n_triangles, found;
	struct MC_triangle **triangle_list, *triangle;
	struct MC_vertex *mc_vertex,  *return_value;

	ENTER(check_mc_vertex);
	/* go through cell triangle vertices and compare */
	found=0;
	return_value=NULL;
	n_triangles=mc_cell->n_triangles[a];
	triangle_list=mc_cell->triangle_list[a];
	if (triangle_list)
	{
		for (i=0;i<n_triangles;i++)
		{
			if (!found)
			{
				triangle=triangle_list[i];
				for (j=0;j<3;j++)
				{
					if (!found)
					{
						mc_vertex=triangle->vertices[j];
						if ((fabs(mc_vertex->coord[0] - v[0]) < MC_EPSILON)&&
							(fabs(mc_vertex->coord[1] - v[1]) < MC_EPSILON)&&
							(fabs(mc_vertex->coord[2] - v[2]) < MC_EPSILON))
						{
							found=1;
							return_value=mc_vertex;
						}
					}
				}
			}
		}
	}
	LEAVE;

	return (return_value);
} /* check_mc_vertex */

static void add_mc_triangle(int i,int j,int k,
	struct MC_iso_surface *mc_iso_surface,int a,float mc_vertices[3][3],int x_min,
	int x_max,int y_min,int y_max,int z_min,int z_max,int mcnx,int mcny,int mcnz,
	int n_scalar_fields)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Add mc_triangle to mc_cell list and add new vertex if unique
==============================================================================*/
{
	int n, nn, ii, jj, kk, i_min, i_max, j_min, j_max, k_min, k_max, found;
	/* pointers to vertices */
	struct MC_vertex *v[3];
	struct MC_triangle *triangle,**temp_triangle_ptrs,**triangle_list,
		**temp_triangle_list;
	struct MC_cell *mc_cell;

	ENTER(add_mc_triangle);
  USE_PARAMETER(x_min);
  USE_PARAMETER(x_max);
  USE_PARAMETER(y_min);
  USE_PARAMETER(y_max);
  USE_PARAMETER(z_min);
  USE_PARAMETER(z_max);
  USE_PARAMETER(mcnz);
  USE_PARAMETER(n_scalar_fields);
	i_min=i_max=i;
	j_min=j_max=j;
	k_min=k_max=k;
	if (i>0)
	{
		i_min=i-1;
	}
	if (i < mc_iso_surface->dimension[0]+1);
	{
		i_max=i+1;
	}
	if (j>0)
	{
		j_min=j-1;
	}
	if (j < mc_iso_surface->dimension[1]+1)
	{
		j_max=j+1;
	}
	if (k>0)
	{
		k_min=k-1;
	}
	if (k < mc_iso_surface->dimension[2]+1)
	{
		k_max=k+1;
	}
	/* step through each vertex */
	for (n=0;n<3;n++)
	{
		found=0;
		for (ii=i_min;ii<=i_max;ii++)
		{
			for (jj=j_min;jj<=j_max;jj++)
			{
				for (kk=k_min;kk<=k_max;kk++)
				{
					if ((NULL !=mc_iso_surface->mc_cells[ii+mcnx*jj+mcnx*mcny*kk])&&
						!found)
					{
						v[n]=check_mc_vertex(
							mc_iso_surface->mc_cells[ii+mcnx*jj+mcnx*mcny*kk],mc_vertices[n],
							a);
						if (NULL !=v[n])
						{
							found=1;
						}
					}
				}
			}
		}
		if (!found)
		{
			if (ALLOCATE(v[n], struct MC_vertex, 1))
			{
				mc_iso_surface->n_vertices++;
				v[n]->n_triangle_ptrs=0;
				v[n]->triangle_ptrs=NULL;
				v[n]->vertex_index=-1;
				for (nn=0;nn<3;nn++)
				{
					v[n]->coord[nn]=mc_vertices[n][nn];
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"add_mc_triangle.   Alloc vertex failed");
			}
		}
	}
	/* allocate triangle */
	if (ALLOCATE(triangle, struct MC_triangle,  1))
	{
		mc_iso_surface->n_triangles++;
		triangle->triangle_list_index=a;
		triangle->triangle_index= -1;
		triangle->vertex_index[0]= -1;
		triangle->vertex_index[1]= -1;
		triangle->vertex_index[2]= -1;
		for (n=0;n<3;n++)
		{
			triangle->vertices[n]=v[n];
			/* initialise structure */
			triangle->env_map[n]=NULL;
			triangle->env_map_index[n]=0;
			triangle->material[n]=NULL;
			for (ii=0;ii<3;ii++)
			{
				triangle->texture_coord[n][ii]=triangle->iso_poly_cop[n][ii]=0;
			}
			/* for each vertex assign back pointers */
			if (ALLOCATE(temp_triangle_ptrs,struct MC_triangle *,
				v[n]->n_triangle_ptrs+1))
			{
				for (ii=0; ii < v[n]->n_triangle_ptrs;ii++)
				{
					temp_triangle_ptrs[ii]=v[n]->triangle_ptrs[ii];
				}
				temp_triangle_ptrs[v[n]->n_triangle_ptrs]=triangle;
				v[n]->n_triangle_ptrs++;
				DEALLOCATE(v[n]->triangle_ptrs);
				v[n]->triangle_ptrs=temp_triangle_ptrs;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"add_mc_triangle.   Alloc triangle_ptrs failed");
			}
		}
		triangle->cell_ptr=mc_iso_surface->mc_cells[i+mcnx*j+mcnx*mcny*k];
		if (triangle->cell_ptr==NULL)
		{
			display_message(ERROR_MESSAGE,"add_mc_triangles.  Cell=NULL");
/*???debug */
printf("ERROR: cell(%d %d %d)=NULL\n", i, j, k);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"add_mc_triangle:  Alloc triangle failed");
	}
	/* update cell list of triangles */
	mc_cell=mc_iso_surface->mc_cells[i+mcnx*j+mcnx*mcny*k];
	if (mc_cell)
	{
		if (ALLOCATE(temp_triangle_list,struct MC_triangle *,
			mc_cell->n_triangles[a]+1))
		{
			triangle_list=mc_cell->triangle_list[a];
			for (ii=0;ii<mc_cell->n_triangles[a];ii++)
			{
				temp_triangle_list[ii]=triangle_list[ii];
			}
			temp_triangle_list[mc_cell->n_triangles[a]]=triangle;
			DEALLOCATE(triangle_list);
			mc_cell->triangle_list[a]=temp_triangle_list;
			mc_cell->n_triangles[a]++;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"add_mc_triangle.   Alloc triangle_list failed");
		}
	}
	LEAVE;
} /* add_mc_triangle */

static void destroy_mc_triangle(struct MC_triangle *triangle,
	struct MC_iso_surface *mc_iso_surface)
/*******************************************************************************
LAST MODIFIED : 28 August 1998

DESCRIPTION :
Destroy mc_triangle
==============================================================================*/
{
	int i, j, k, delete_index;
	struct MC_vertex *v;
	struct MC_triangle **temp_triangle_ptrs;

	ENTER(destroy_mc_triangle);
	/* delete vertex list links - if vertex shares no other triangles, delete
		it */
	for (i=0;i<3;i++)
	{
		v=triangle->vertices[i];
		/* search through vertex list and delete entry - if no other entries,
			delete vertex */
		for (j=0;j < v->n_triangle_ptrs; j++)
		{
			if (v->triangle_ptrs[j]==triangle)
			{
				delete_index=j;
			}
		}
		if (1==v->n_triangle_ptrs)
		{
			mc_iso_surface->n_vertices--;
			/* delete vertex as this is the last one */
			DEALLOCATE(v->triangle_ptrs);
			DEALLOCATE(v);
		}
		else
		{
			/* copy shortened list minus current triangle */
			k=0;
			if (ALLOCATE(temp_triangle_ptrs,struct MC_triangle *,
				v->n_triangle_ptrs-1))
			{
				for (j=0;j<v->n_triangle_ptrs;j++)
				{
					/* leave out one to be deleted */
					if (j!=delete_index)
					{
						temp_triangle_ptrs[k]=v->triangle_ptrs[j];
						k++;
					}
				}
				DEALLOCATE(v->triangle_ptrs);
				v->triangle_ptrs=temp_triangle_ptrs;
				v->n_triangle_ptrs--;
			}
			else
			{
				display_message(ERROR_MESSAGE,"destroy_mc_triangle.  Alloc failed");
			}
		}
	}
	DEALLOCATE(triangle);
	LEAVE;
} /* destroy_mc_triangle */

static void destroy_mc_triangle_field_list(struct MC_cell *mc_cell,
	struct MC_triangle **triangle_list,int n_triangles,
	struct MC_iso_surface *mc_iso_surface)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Destroy triangle list for particular scalar field or face.
==============================================================================*/
{
	int i;

	ENTER(destroy_mc_triangle_field_list);
  USE_PARAMETER(mc_cell);
	if (triangle_list && n_triangles > 0)
	{
		for (i=0;i < n_triangles;i++)
		{
			if (triangle_list[i])
			{
				destroy_mc_triangle(triangle_list[i], mc_iso_surface);
				mc_iso_surface->n_triangles--;
			}
		}
		DEALLOCATE(triangle_list);
	}
	LEAVE;
} /* destroy_mc_triangle_field_list */

static double rqs(struct VT_scalar_field *scalar_field,int i,int j,int k)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Gives requested scalar value at a node in a volume_texture
==============================================================================*/
{
int nx,ny,nz;

/* adjust dimension for nodes (1 greater the cells) */
nx=scalar_field->dimension[0]+1;
ny=scalar_field->dimension[1]+1;
nz=scalar_field->dimension[2]+1;

/* if closed surface, indices will be out of range on border, which is effectively
covered in zeros for closure */

if ((i==-1) || (j==-1) || (k==-1) || (i==nx) || (j==ny) || (k==nz))
	{
	return(0.0);
	}
else
	{
	return(scalar_field->scalar[i + j*nx + k*nx*ny]);
	}
} /* rqs */

#if defined (OLD_CODE)
static void rqsg(struct VT_volume_texture *texture,int i,int j,int k,
	double grad[3])
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Gives requested scalar gradient at a node in a volume_texture
==============================================================================*/
{
int nx,ny,nz,n;

nx=texture->dimension[0]+1;
ny=texture->dimension[1]+1;
nz=texture->dimension[2]+1;

/* if closed surface, indices will be out of range on border, which is effectively
covered in zeros for closure */

if ((i==-1) || (j==-1) || (k==-1) || (i==nx) || (j==ny) || (k==nz))
	{
	for (n=0;n<3;n++)
		{
		grad[n]=0.0;
		}
	}
else
	{
	for (n=0;n<3;n++)
		{
		grad[n]=(texture->global_texture_node_list[i + j*nx + k*nx*ny])->scalar_gradient[n];
		}
	}
} /* rqsg */
#endif /* defined (OLD_CODE) */

static void dblcon(struct VT_scalar_field *scalar_field,int nx,int ny,int nz,
	int ic,int jc,int kc,int iface,double cl,double *grads)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Sorts out the tricky 'double contour' cases - returns gradient in grads
==============================================================================*/
{
int i,in,jn,kn;
int icell;
int border;
double f1,f2,f3,f4,a,b,c,d;
double fn[8];

/* adjust nx etc if using virtual dim[n+1] cube for closed surfaces. I am not sure
about this */
if (scalar_field->dimension[0] !=nx) nx--;
if (scalar_field->dimension[1] !=nx) ny--;
if (scalar_field->dimension[2] !=nx) nz--;
/*--------*/

for (i=1;i<=4;i++)
	{
	/*determine cell indices of an adjacent cell */
	icell=adjcel[iface -1][i -1];
	in=ic;
	jn=jc;
	kn=kc;
	border=0;
	switch (icell)
		{
		case(1):
			in=in-1;
			if (in <1)
				{
				border=1;
				}
			break;
		case(2):
			in=in+1;
			if (in >nx)
				{
				border=1;
				}
			break;
		case(3):
			jn=jn-1;
			if (jn <1)
				{
				border=1;
				}
			break;
		case(4):
			jn=jn+1;
			if (jn >ny)
				{
				border=1;
				}
			break;
		case(5):
			kn=kn-1;
			if (kn <1)
				{
				border=1;
				}
			break;
		case(6):
			kn=kn-1;
			if (kn >nz)
				{
				border=1;
				}
			break;
		default:
			break;
		}
	if (!border)
		{
		/* get scalar values for given cell in fn here */
			fn[0]=rqs(scalar_field,in-1,jn-1,kn-1);
			fn[1]=rqs(scalar_field,in,jn-1,kn-1);
			fn[2]=rqs(scalar_field,in-1,jn,kn-1);
			fn[3]=rqs(scalar_field,in,jn,kn-1);
			fn[4]=rqs(scalar_field,in-1,jn-1,kn);
			fn[5]=rqs(scalar_field,in,jn-1,kn);
			fn[6]=rqs(scalar_field,in-1,jn,kn);
			fn[7]=rqs(scalar_field,in,jn,kn);
		}

	f1=fn[facnod[iface -1][1 -1]];
	f2=fn[facnod[iface -1][2 -1]];
	f3=fn[facnod[iface -1][3 -1]];
	f4=fn[facnod[iface -1][4 -1]];
	if (!(  ((f1>cl) && (f3>cl) && (f2 < cl) && (f4 < cl)) ||
		((f1<cl) && (f3<cl) && (f2 > cl) && (f4 > cl)) ))
		{
		/* evaluate the sign of the gradient of the adjacent face */
		a=f1 - cl;
		b=f4 - f1;
		c=f2 - f1;
		d=f1 - f2 + f3 - f4;
		*grads=a*d - b*c;
		return;
		}
	}
/* all adjacent faces have double contours */
*grads=1.0;
return;
} /* dblcon */

static int rqc(struct VT_vector_field *coordinate_field,int i,int j,int k,
	double coord[3])
/*******************************************************************************
LAST MODIFIED : 21 December 1995

DESCRIPTION :
Gives requested coordinates of a node in a volume_texture returns 1 if value on
border
==============================================================================*/
{
	int nx,ny,nz,n;
	int is_border_face=0;
	int x1,x2,y1,y2,z1,z2;

	ENTER(rqc);
	/* these are variables which are used to deal with a virtual border (1 cell
		thick) used when calculating a closed surface isosurface. If the coordinate
		value does not exist, then one is created by extrapolating using the width
		of the cells at the edge of the real cube.  The formula
		coord(x)=2*coord(x1)-coord(x2)
		is used which produces coord(x) if x1=x2 and the desired extrapolation if
		not */
	x1=x2=i;
	y1=y2=j;
	z1=z2=k;
	nx=coordinate_field->dimension[0]+1;
	ny=coordinate_field->dimension[1]+1;
	nz=coordinate_field->dimension[2]+1;
	/* if closed surface, indices will be out of range on border.  To achieve a
		flat surface on the borders, the virtual border cells have no thickness and
		hence the coordinate values of the interior adjacent cells are returned */
	if (i==-1)
	{
		x1=0;x2=1;
		is_border_face=1;
	}
	if (j==-1)
	{
		y1=0;y2=1;
		is_border_face=1;
	}
	if (k==-1)
	{
	z1=0;z2=1;
	is_border_face=1;
	}
	if (i==nx)
	{
		x1=nx-1;x2=nx-2;
		is_border_face=1;
	}
	if (j==ny)
	{
		y1=ny-1;y2=ny-2;
		is_border_face=1;
	}
	if (k==nz)
	{
		z1=nz-1;z2=nz-2;
		is_border_face=1;
	}
	/* for time being, the coordinates are just for an evenly spaced rectangular
		grid.  This probably should be changed so irregular grids, or curvilinear
		grids by having a node coordinate field */
#if defined (OLD_CODE)
	val[0]=(double) i;
	val[1]=(double) j;
	val[2]=(double) k;
#endif /* defined (OLD_CODE) */
	/*???MS.  If border get the extra point by extrapolating the point from the
		internal values */
	for (n=0;n<3;n++)
	{
/*    coord[n]=val[n]/((double) texture->dimension[n]) *
			(texture->ximax[n]-texture->ximin[n]) + texture->ximin[n]; */
		coord[n]=2.0 * coordinate_field->vector[3*(x1 + y1*nx + z1*nx*ny)+n] -
			coordinate_field->vector[3*(x2 + y2*nx + z2*nx*ny)+n];
	}
	LEAVE;

	return(is_border_face);
} /* rqc */

static struct Graphical_material *rqm(struct VT_volume_texture *texture,int i,
	int j,int k)
/*******************************************************************************
LAST MODIFIED : 19 November 1994

DESCRIPTION :
Gives requested material index at a node in a volume_texture
==============================================================================*/
{
	int nx,ny,nz;

	nx=texture->dimension[0]+1;
	ny=texture->dimension[1]+1;
	nz=texture->dimension[2]+1;
	/* if closed surface, indices will be out of range on border, which is
		effectively covered in zeros for closure */
	if (i <=-1) i=0;
	if (j <=-1) j=0;
	if (k <=-1) k=0;
	if (i >=nx) i=nx-1;
	if (j >=ny) j=ny-1;
	if (k >=nz) k=nz-1;

	return((texture->global_texture_node_list[i + j*nx + k*nx*ny])->
		dominant_material);
} /* rqm */

#if defined (OLD_CODE)
static void rqm_cop(double cop[3],struct VT_volume_texture *texture,int i,int j,
	int k)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Gives requested material cop at a node in a volume_texture
==============================================================================*/
{
	int nx,ny,nz,l;

	nx=texture->dimension[0]+1;
	ny=texture->dimension[1]+1;
	nz=texture->dimension[2]+1;
	/* if closed surface, indices will be out of range on border, which is
		effectively covered in zeros for closure */
	if (i <=-1) i=0;
	if (j <=-1) j=0;
	if (k <=-1) k=0;
	if (i >=nx) i=nx-1;
	if (j >=ny) j=ny-1;
	if (k >=nz) k=nz-1;
	for (l=0;l<3;l++)
	{
		cop[l]=(texture->global_texture_node_list[i + j*nx + k*nx*ny])->
			cop[l];
	}
} /* rqm_cop */
#endif /* defined (OLD_CODE) */

static struct Graphical_material *rqmc(struct VT_volume_texture *texture,int i,
	int j,int k)
/*******************************************************************************
LAST MODIFIED : 19 November 1994

DESCRIPTION :
Gives requested material index at a cell in a volume_texture
==============================================================================*/
{
	int nx,ny,nz;

	nx=texture->dimension[0];
	ny=texture->dimension[1];
	nz=texture->dimension[2];

	/* if closed surface, indices will be out of range on border, which is
		effectively covered in zeros for closure */
	if (i<-1 || i > nx+1 || j<-1 || j>ny+1 || k<-1 || k>nz+1)
	{
		display_message(ERROR_MESSAGE,"rqmc");
		return (NULL);
	}
	else
	{
	if (i==-1) i++;
	if (j==-1) j++;
	if (k==-1) k++;
	if (i==nx+1) i--;
	if (j==ny+1) j--;
	if (k==nz+1) k--;

	if (i==nx) i--;
	if (j==ny) j--;
	if (k==nz) k--;

	return((texture->texture_cell_list[i + j*nx + k*nx*ny])->material);
	}
} /* rqmc */

static void rqmc_cop(double *cop,struct VT_volume_texture *texture,int i,int j,
	int k)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Gives requested material index at a cell in a volume_texture
==============================================================================*/
{
	int nx,ny,nz,l;

	nx=texture->dimension[0];
	ny=texture->dimension[1];
	nz=texture->dimension[2];

	/* if closed surface, indices will be out of range on border, which is
		effectively covered in zeros for closure */
	if (i<-1 || i > nx+1 || j<-1 || j>ny+1 || k<-1 || k>nz+1)
	{
		display_message(ERROR_MESSAGE,"rqmc_cop");
			for (l=0;l<3;l++)
		{
		cop[l]=0;
		}
	}
	else
	{
	if (i==-1) i++;
	if (j==-1) j++;
	if (k==-1) k++;
	if (i==nx+1) i--;
	if (j==ny+1) j--;
	if (k==nz+1) k--;

	if (i==nx) i--;
	if (j==ny) j--;
	if (k==nz) k--;

	for (l=0;l<3;l++)
	{
		cop[l]=(texture->texture_cell_list[i + j*nx + k*nx*ny])->cop[l];
	}
	}
} /* rqmc_cop */

static double rqsc(struct VT_volume_texture *texture,int i,int j,int k)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Gives requested scalar value at a cell in a volume_texture
==============================================================================*/
{
int nx,ny,nz;

nx=texture->dimension[0];
ny=texture->dimension[1];
nz=texture->dimension[2];

/* if closed surface, indices will be out of range on border, which is effectively
covered in zeros for closure */

if ((i==-1) || (j==-1) || (k==-1) || (i==nx+1) || (j==ny+1) || (k==nz+1))
	{
	return(0);
	}
else
	{
	if (i==nx) i--;
	if (j==ny) j--;
	if (k==nz) k--;
	return((texture->texture_cell_list[i + j*nx + k*nx*ny])->scalar_value);
	}
} /* rqsc */

#if defined (OLD_CODE)
static double rqclip(struct VT_scalar_field *clip_field,int i,int j,int k)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Gives requested scalar clipping fn value at a node in a volume_texture
==============================================================================*/
{
int nx,ny,nz;
/* adjust dimension for nodes (1 greater the cells) */
nx=clip_field->dimension[0]+1;
ny=clip_field->dimension[1]+1;
nz=clip_field->dimension[2]+1;

if (i==-1) i++;
if (j==-1) j++;
if (k==-1) k++;
if (i==nx) i--;
if (j==ny) j--;
if (k==nz) k--;

return(clip_field->scalar[i + j*nx + k*nx*ny]);

} /* rqclip */
#endif /* defined (OLD_CODE) */

static struct Environment_map *rqenvc(struct VT_volume_texture *texture,int i,
	int j,int k)
/*******************************************************************************
LAST MODIFIED : 25 July 1995

DESCRIPTION :
Gives requested environment map index at a cell in a volume_texture
==============================================================================*/
{
	int nx,ny,nz;

	nx=texture->dimension[0];
	ny=texture->dimension[1];
	nz=texture->dimension[2];

	/* if closed surface, indices will be out of range on border, which is
		effectively covered in zeros for closure */
	if (i<-1 || i > nx+1 || j<-1 || j>ny+1 || k<-1 || k>nz+1)
	{
		display_message(ERROR_MESSAGE,"rqenvc");
		return (NULL);
	}
	else
	{
	if (i==-1) i++;
	if (j==-1) j++;
	if (k==-1) k++;
	if (i==nx+1) i--;
	if (j==ny+1) j--;
	if (k==nz+1) k--;

	if (i==nx) i--;
	if (j==ny) j--;
	if (k==nz) k--;

	return((texture->texture_cell_list[i + j*nx + k*nx*ny])->env_map);
	}
} /* rqenvc */

static int scalar_interpolation(struct Triangle *triangle,struct Cell_fn *c_fn)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
calculates interpolated scalar value for trilinear fn, using actual coordinates
returns 1 if unsuccessful
==============================================================================*/
{

int i;
double p_xi[3];
double *xc,*yc,*zc,*fn;


if (triangle && c_fn )
	{

	xc=c_fn->xc;
	yc=c_fn->yc;
	zc=c_fn->zc;
	fn=c_fn->fn;

	/* calculate xi parameters to perform trilinear
	interpolation in f space */

	for (i=0;i<3;i++)   /* vertices of trinangle */
		{
		if (xc[7]==xc[0])
			p_xi[0]=0.5;
		else
			p_xi[0]=(triangle->v[i][0]-xc[0])/(xc[7]-xc[0]);

		if (yc[7]==yc[0])
			p_xi[1]=0.5;
		else
			p_xi[1]=(triangle->v[i][1]-yc[0])/(yc[7]-yc[0]);

		if (zc[7]==zc[0])
			p_xi[2]=0.5;
		else
			p_xi[2]=(triangle->v[i][2]-zc[0])/(zc[7]-zc[0]);

		triangle->trilinear_int[i]=(1.0-p_xi[2])*((1.0-p_xi[1])*((1.0-p_xi[0])*fn[0]+p_xi[0]*fn[1])
			+ p_xi[1]*((1.0-p_xi[0])*fn[2]+p_xi[0]*fn[3]))
			+ p_xi[2]*((1.0-p_xi[1])*((1.0-p_xi[0])*fn[4]+p_xi[0]*fn[5])
			+ p_xi[1]*((1.0-p_xi[0])*fn[6]+p_xi[0]*fn[7]));
		}
	return(0);
	}
else
	{
	printf("ERROR: scalar interpolation , NULL values\n");
	return(1);
	}
} /* scalar_interpolation */

static struct Triangle *make_triangle(double vertices[3][3],int clip_history,
	int clip_history2)
/*******************************************************************************
LAST MODIFIED : 21 December 1995

DESCRIPTION :
Creates and sets up triangle data structure, including plane eqn parameters.
==============================================================================*/
{
int i,j;
double v1[3],v2[3];
struct Triangle *triangle;

if (ALLOCATE(triangle,struct Triangle,1))
	{
	for (i=0;i<3;i++)
		{
		for (j=0;j<3;j++)
			{
			triangle->v[i][j]=vertices[i][j];
			}
		/* material at vertex */
		}
	for (i=0;i<3;i++)
		{
		v1[i]=triangle->v[1][i]-triangle->v[0][i];
		v2[i]=triangle->v[2][i]-triangle->v[0][i];
		}
	/* calculate normal */
	if (cross_product(v1,v2,triangle->n))
		{
		/* ax+by+cz+d=0 */
		triangle->d=-(triangle->n[0]*triangle->v[0][0]
			+triangle->n[1]*triangle->v[0][1]
			+triangle->n[2]*triangle->v[0][2]);
		}
	else
		{
		if (debug_flag)
			printf("** ERROR ** make_triangle : bad cross product\n");
		return(NULL);
		}
	triangle->clip_history=clip_history;
	triangle->clip_history2=clip_history2;
	return(triangle);
	}
else
	{
	printf("*** ERROR *** make_triangle: - memory allocate failed\n");
	return(NULL);
	}
}  /* make_triangle */

static void recurse_clip(struct Triangle *triangle,int n_clipping_triangles,
	struct Triangle **clip_triangles,int n_clip)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Clips a triangle against a clipping triangle, retrianglates it and recursively
clips the parts. If completely clipped after module global n_clipping_triangles,
resultant triangles are stored in module global clipped_triangle_list.
==============================================================================*/
{
double cutoff=0;
int i,j,k;
struct Triangle *clip_triangle;
double new_v[3][3];
int new_m[3];
double v1[3],v2[3],v3[3],va[3],vb[3],vc[3],q[3],sa,sb,sc;
/* temporary vertices for splitting triangle */
double temp_v[9][3],vertex_value[3];
/* temp_v[9..={(t1) v1,2,3,4,5,6, (t2) v1,2,3}*/
/* flag for whether edge clipped */
int edge_count1,edge_count2;
int clip_case;
if (debug_flag)
	printf("in recurse_clip. level=%d\n",n_clip);

if (triangle==NULL)
	{
	if (debug_flag)
		printf("recurse_clip : NULL triangle, aborting\n");
	return;
	}

/* if no more clipping planes then done */
/* examine triangle to see if not 1D */
for (i=0;i<3;i++)
	vertex_value[i]=0;

if (vectors_equal(triangle->v[0],triangle->v[1],AREA_ACC) || vectors_equal(triangle->v[1],triangle->v[2],AREA_ACC) ||
	vectors_equal(triangle->v[0],triangle->v[2],AREA_ACC) )
	{
	if (debug_flag)
		printf("****** triangle has no area,aborting  ***********\n");
	DEALLOCATE(triangle);
	return;
	}

if (n_clip >=n_clipping_triangles)
	{
	if (debug_flag)
		printf("n_clip=n_clipping_triangles\n");
	if (debug_flag)
		printf("Triangle->clip_history=%d %d \n",triangle->clip_history,triangle->clip_history2);


	if (triangle->clip_history==-1)
		{
		/* means no clip or meaningful comparison has occurred*/
		/* store triangle in out_list depending on trilinear values at vertices */
		scalar_interpolation(triangle,&cell_fn);
		if (cell_fn.dir*(triangle->trilinear_int[0]+triangle->trilinear_int[1]
			+triangle->trilinear_int[2])/3.0 > cell_fn.dir*cell_fn.isovalue)
			{
			clipped_triangle_list[n_clipped_triangles]=triangle;
			if (debug_flag)
				printf("++ triangle stored (t_int)=#%d ++++++\n",n_clipped_triangles);

			n_clipped_triangles++;
			return;
			}
		else
			{
			DEALLOCATE(triangle);
			return;
			}
		}

	if (triangle->clip_history==0)
		{
		/* store triangle in out_list and exit */
		clipped_triangle_list[n_clipped_triangles]=triangle;
		if (debug_flag)
			printf("++++++ triangle stored=#%d ++++++\n",n_clipped_triangles);
		if (debug_flag)
			{
			printf("vertices stored: ");
			for (j=0;j<3;j++)
				printf(" %f %f %f / ",triangle->v[j][0],triangle->v[j][1],triangle->v[j][2]);
			printf("\n");
			}

		n_clipped_triangles++;
		return;
		}
	else    /* it has had no intersections, but is below clipping surfaces */
		{
		if (debug_flag)
			printf("not stored\n");
		DEALLOCATE(triangle);
		return;
		}
	}
else
	{
	/* check for intersection between triangle and clip_triangle */
	triangle->clip_history2=0;
	clip_triangle=clip_triangles[n_clip];

	if (clip_triangle==NULL)
		{
		/* go to next one */
		printf("CLIP_TRIANGLE=NULL\n");
		recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
		return;
		}
	if (debug_flag)
		printf("nclip=%d\n",n_clip);

	for (i=0;i<3;i++)
		{
		for (j=0;j<3;j++)
			{
			temp_v[i][j]=triangle->v[i][j];
			}
		}

	edge_count1=0; edge_count2=0;
	/* go through edges on triangle */
	for (i=0;i<3;i++)
		{
		for (j=0;j<3;j++)
			{
			va[j]=triangle->v[triangle_edge_list[i][0]][j];
			vb[j]=triangle->v[triangle_edge_list[i][1]][j];
			vc[j]=triangle->v[triangle_edge_list[i+1][1]][j];
			}
		if (vectors_equal(va,vb,ACC3))
			printf("^^^^^^^^^^^^^^va=vb\n");

		if (debug_flag)
			{
			printf("va,vb,vc=%f %f %f | %f %f %f | %f %f %f\n",va[0],va[1],va[2],vb[0],vb[1],vb[2],vc[0],vc[1],vc[2]);
			printf("clip_triangle->d %f\n",clip_triangle->d);
			printf("clip_trinagle->n %f %f %f \n",clip_triangle->n[0],clip_triangle->n[1],clip_triangle->n[2]);
			printf("clip_trinagle->v : ");
			for (j=0;j<3;j++)
				printf(" %f %f %f | ",clip_triangle->v[j][0],clip_triangle->v[j][1],clip_triangle->v[j][2]);
			printf("\n");
			}

		/* test to see if on edge of cube */
		if ((va[0]==xc[0] && vb[0]==xc[0]) || (va[0]==xc[7] && vb[0]==xc[7])
		|| (va[1]==yc[0] && vb[1]==yc[0]) || (va[1]==yc[7] && vb[1]==yc[7])
		|| (va[2]==zc[0] && vb[2]==zc[0]) || (va[2]==zc[7] && vb[2]==zc[7]))
			{
			if (debug_flag)
				printf("<<<<<<<<<<<<<Edge on edge of cube>>>>>>>>>>>>>>>>>>\n");
			cutoff=-INTERSECTION_ACC;
			}
		else
			cutoff=INTERSECTION_ACC;
		/* sa=ax + by + cz + d, where x,y,z are coords in triangle
		and a,b,c,d are plane eqn values from the clipping triangle */
		sa=   clip_triangle->n[0]*va[0] + clip_triangle->n[1]*va[1]
			+ clip_triangle->n[2]*va[2] + clip_triangle->d;
		sb=   clip_triangle->n[0]*vb[0] + clip_triangle->n[1]*vb[1]
			+ clip_triangle->n[2]*vb[2] + clip_triangle->d;
		sc=  clip_triangle->n[0]*vc[0] + clip_triangle->n[1]*vc[1]
			+ clip_triangle->n[2]*vc[2] + clip_triangle->d;


		/* store vertex values to decide what parts of triangle to clip */
		vertex_value[i]=sa;
		if (sa==0 && sb==0 && sc==0)
			{
			if (debug_flag)
				printf("@@@@@@@@ sa=sb=sc=0 @@@@@@@@\n");
			}
		if (debug_flag)
					printf("sa,sb=%f %f\n",sa,sb);
		if ((sa >=0 && sb <=0) || (sa <=0 && sb >=0)) /* edges passes through plane of clipping poly */
			{
			if (sa==0 && sb==0) /* an edge of a clipping trinagle lies along the triangle */
				{
				if (debug_flag)
					printf("sa=sb=0 : treating as non_intersecting\n");
				if (sc > 0)   /* >=?? */
					{
					if (debug_flag)
						printf("sa=sb=0: above clip\n");
					triangle->clip_history=1;
					recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
					return;
					}
				else
					{
					if (debug_flag)
						printf("sa=sb=0: above clip\n");
					triangle->clip_history=0;
					recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
					return;
					}
				}
			for (j=0;j<3;j++)
				{
				/* q is the point of intersection of the edge and the clipping triangle plane */
				q[j]=va[j] + sa/(sa-sb) * (vb[j]-va[j]);
				v1[j]=clip_triangle->v[0][j] - q[j];
				v2[j]=clip_triangle->v[1][j] - q[j];
				v3[j]=clip_triangle->v[2][j] - q[j];
				/* store as intermediate vertices 3,4,5 (between 0,1,2) */
				temp_v[i+3][j]=q[j];
				}
			if (scalar_triple_product(v1,v2,clip_triangle->n) >=cutoff /* intersection is inside triangle */
					&&  scalar_triple_product(v2,v3,clip_triangle->n) >=cutoff
					&&  scalar_triple_product(v3,v1,clip_triangle->n) >=cutoff)
				{
				if (debug_flag)
					printf("stp:  %f,%f,%f\n",
						scalar_triple_product(v1,v2,clip_triangle->n),
								scalar_triple_product(v2,v3,clip_triangle->n),
								scalar_triple_product(v3,v1,clip_triangle->n));
						if (scalar_triple_product(v1,v2,clip_triangle->n)==0
							||  scalar_triple_product(v2,v3,clip_triangle->n)==0
							||  scalar_triple_product(v3,v1,clip_triangle->n)==0)
							{
							if (debug_flag)
								printf("one of stp=0\n");
							}
				/* the point of intersection has position vector q */
				/* this is the case were the clip is on an edge of triangle,
					so this will become a new vertex */
				edge_count1++;
				}
			}  /* if */

		}  /* i */
	if (debug_flag)
		printf("edge_count1=%d\n",edge_count1);

	if (edge_count1 < 2)   /* swap triangles to find other edge which passes thru */
		{ /* (there can only be 2 so inly enter if necessary) */
		for (i=0;i<3;i++)
			{
			for (j=0;j<3;j++)
				{
				va[j]=clip_triangle->v[triangle_edge_list[i][0]][j];
				vb[j]=clip_triangle->v[triangle_edge_list[i][1]][j];
				}
		if ((va[0]==xc[0] && vb[0]==xc[0]) || (va[0]==xc[7] && vb[0]==xc[7])
		|| (va[1]==yc[0] && vb[1]==yc[0]) || (va[1]==yc[7] && vb[1]==yc[7])
		|| (va[2]==zc[0] && vb[2]==zc[0]) || (va[2]==zc[7] && vb[2]==zc[7]))
			{
			if (debug_flag)
				printf("<<<<<<<<<<<<<Edge on edge of cube>>>>>>>>>>>>>>>>>>\n");
			cutoff=-INTERSECTION_ACC;
			}
		else
			cutoff=INTERSECTION_ACC;

			/* sa=ax + by + cz + d, where x,y,z are coords in triangle
			and a,b,c,d are plane eqn values from the clipping triangle */
			sa=   triangle->n[0]*va[0] + triangle->n[1]*va[1]
				+ triangle->n[2]*va[2] + triangle->d;
			sb=   triangle->n[0]*vb[0] + triangle->n[1]*vb[1]
				+ triangle->n[2]*vb[2] + triangle->d;

			if ((sa >=0 && sb <=0) || (sa <=0 && sb >=0)) /* edges passes through plane of clipping poly */
				{
				if (debug_flag)
					printf("clip sa,sb=%f %f\n",sa,sb);
				if (sa==0 && sb==0) /* this means clip triangle edeg lies on normal trinagle*/
					{
					/*=> treat as intersecting, set edge_count2 > 0 */
					if (debug_flag)
						printf("clip sa=sb=0, treating as non intersecting \n");
					/*edge_count2++;*/
					}
				else
					{
					for (j=0;j<3;j++)
						{
						q[j]=va[j] + sa/(sa-sb) * (vb[j]-va[j]);
						v1[j]=triangle->v[0][j] - q[j];
						v2[j]=triangle->v[1][j] - q[j];
						v3[j]=triangle->v[2][j] - q[j];
						}
					if (debug_flag)
						printf("stp: clip : %f,%f,%f\n",
							scalar_triple_product(v1,v2,triangle->n),
									scalar_triple_product(v2,v3,triangle->n),
									scalar_triple_product(v3,v1,triangle->n));
					if (scalar_triple_product(v1,v2,triangle->n) >=cutoff  /* intersection is inside triangle */
								&& scalar_triple_product(v2,v3,triangle->n) >=cutoff
								&& scalar_triple_product(v3,v1,triangle->n) >=cutoff
								)
						{
						if (debug_flag)

						/* the point of intersection has position vector q */
						/* this is the case were the clip is on an edge of triangle,
						so this will become a new vertex */
						if (scalar_triple_product(v1,v2,triangle->n)==0
									||  scalar_triple_product(v2,v3,triangle->n)==0
									||  scalar_triple_product(v3,v1,triangle->n)==0)
									{
									if (debug_flag)
										printf("clip: one of stp=0\n");
									}

						edge_count2++;
						}
					} /* if */
				} /* if sa ,sb >, < 0 && || ... */
			} /* i */
		} /* if edge_count1 < 2 */
	if (debug_flag)
		printf("edge_count2=%d\n",edge_count2);


	if (debug_flag)
		{
		printf("temp_v: \n");
		for (j=0;j<6;j++)
			{
			for (k=0;k<3;k++)
				{
				printf(" %f ",temp_v[j][k]);
				}
			printf("\n");
			}
		}

	/* if edge_count=0=> no intersection, but surface may be completely clipped.
	check by comparing vertex positions to clipping planes. If clip_history > 0, means
	no intersection has been encountered, and it lies below the clipping planes, so therefore
	do not add it to out_list. If it is clipped by another plane, this status is removed */

	if (edge_count1 + edge_count2==0 )
		{
		/* check a vertex to see if under or over clip plane  v >0=> above cut plane=> maybe clip it off*/

		if (vertex_value[0]==0 && vertex_value[1]==0 && vertex_value[2]==0)
			{
			printf("!!!!!!!!! vertex values all equal zero !!!!!!!!!\n");
			/* treat as non_intersecting*/
			if (triangle->clip_history==-1)
				{
				triangle->clip_history=0;
				}


			recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
			return;

			}
		if (vertex_value[0]>=0 && vertex_value[1]>=0 && vertex_value[2] >=0)
			{
			if (debug_flag)
				printf(" v.v >=0, non_intersecting (above clip plane)\n");

			/* only change if unaltered by any other polygon */
			if (triangle->clip_history==-1)
				{
				triangle->clip_history=1;
				}
			recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
			return;
			}
		else
			{
			if (debug_flag)
				printf(" v.v < 0,non_intersecting (below clip plane)\n");

			if (triangle->clip_history==-1)
				{
				triangle->clip_history=0;
				}

			recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
			return;
			}
		}
	else
		{
		if (debug_flag)
			printf("edge_count1+2 > 0 (=%d+%d)=> some kind of clip to occur\n",edge_count1,edge_count2);
		if (edge_count1 + edge_count2 < 2)  /* if non_sensible value, get out */
			{
			/*recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
			return;*/
			}
		triangle->clip_history=0 ;    /* some kind of clip to occur */

		/* check vertex_values. If two are below clipping plane the we get one resulting
		triangle. If one only then retriangulate resulting quadrilateral */
		clip_case=0;
		if (vertex_value[0] >=0)
			clip_case +=1;
		if (vertex_value[1] >=0)
			clip_case +=2;
		if (vertex_value[2] >=0)
			clip_case +=4;
		if (debug_flag)
			printf("vertex_values (%f,%f,%f) clip_case %d\n",vertex_value[0],vertex_value[1],vertex_value[2],clip_case);

		if (clip_case >6 || clip_case <1)
			if (debug_flag)
				printf("$$$$$$$$$$$ HEINIOUS ERROR DUDE - CLIP CASE !!!!\n");
		if (clip_case==7)
			{
			triangle->clip_history=1;
			recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
			return;
			}
		if (clip_case==0)
			{
			triangle->clip_history=0;
			recurse_clip(triangle,n_clipping_triangles,clip_triangles,n_clip+1);
			return;
			}
		for (i=0;i<retriangulation_list[clip_case][0];i++)
			{
			for (j=0;j<3;j++)
				{
				for (k=0;k<3;k++)
					{
					new_v[j][k]=
					temp_v[retriangulation_list[clip_case][j+1+3*i]-1][k];
					/* +1 to avoid count bit, -1 as list has counting # indices */
					}
				}
			if (debug_flag)
				printf("below clip plane \n");

			recurse_clip(make_triangle(new_v,0,0),n_clipping_triangles,clip_triangles,n_clip+1);
			} /* i*/

		if (debug_flag)
			printf("processing part above clip plane\n");

		for (i=0;i<retriangulation_list[7-clip_case][0];i++)
			{
			for (j=0;j<3;j++)
				{
				for (k=0;k<3;k++)
					{
					/* take complement */
					new_v[j][k]=
					temp_v[retriangulation_list[7-clip_case][j+1+3*i]-1][k];
					/* +1 to avoid count bit, -1 as list has counting # indices */
					}
				}
			/* taint triangle as could be totally severed already */
			if (debug_flag)
				printf("above clip plane\n");
			if (debug_flag)
			printf("new_m=%d %d %d  (k=%d)\n",new_m[0],new_m[1],new_m[2],k);

			recurse_clip(make_triangle(new_v,1,1),n_clipping_triangles,clip_triangles,n_clip+1);

			}


		/* deallocate memory assoc with triangle */
		if (triangle)
			DEALLOCATE(triangle);

		} /* if edge_count */
	} /* if nclip */
} /* recurse_clip */

static int add_triangle_to_vertex_ptrlist(struct MC_vertex *vertex,
	struct MC_triangle *triangle)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Adds the given triangle in the triangle pointer list of the given vertex.
==============================================================================*/
{
	int k,return_code;
	struct MC_triangle **temp_triangle_ptrs;

	ENTER(add_triangle_to_vertex_ptrlist);
	return_code=0;
	if (vertex&&triangle)
	{
		if (ALLOCATE(temp_triangle_ptrs,struct MC_triangle *,
			vertex->n_triangle_ptrs+1))
		{
			for (k=0;k<vertex->n_triangle_ptrs;k++)
			{
				temp_triangle_ptrs[k]=vertex->triangle_ptrs[k];
			}
			temp_triangle_ptrs[k]=triangle;
			vertex->n_triangle_ptrs++;
			DEALLOCATE(vertex->triangle_ptrs);
			vertex->triangle_ptrs=temp_triangle_ptrs;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"add_triangle_to_vertex_ptrlist.  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_triangle_to_vertex_ptrlist.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* add_triangle_to_vertex_ptrlist */

static void remove_triangle_from_vertex_ptrlist( struct MC_vertex *vertex,
	struct MC_triangle *triangle )
/*******************************************************************************
LAST MODIFIED : 30 January 1998

DESCRIPTION :
Finds the pointer to the given triangle in the vertex's triangle pointer list
and removes it.  Reports an error if the triangle is not found.
==============================================================================*/
{
	int found, k;

	k = 0;
	found = 0;
	while ( k < vertex->n_triangle_ptrs && !found)
	{
		if ( vertex->triangle_ptrs[k] == triangle )
		{
			found = 1;
		}
		else
		{
			k++;
		}
	}
	if ( found )
	{
		vertex->n_triangle_ptrs--;
		while ( k < vertex->n_triangle_ptrs )
		{
			vertex->triangle_ptrs[k] = vertex->triangle_ptrs[k+1];
			k++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"remove_triangle_from_vertex_ptrlist.  Triangle ptr %d not found in vertex %d.", triangle->triangle_index, vertex->vertex_index );
	}
} /* remove_triangle_from_vertex_ptrlist */

static void retriangulate(struct MC_iso_surface *iso_surface,int i,
	int n_triangles,struct MC_vertex *i_vertex, struct MC_triangle **triangles,
	int *ordered_triangles, int *central_vertex,
	struct MC_triangle **new_triangles, int *new_triangle_count,
	struct MC_triangle **discarded_triangles, int *discarded_triangle_count)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Retriangulates hole, removing vertex.
==============================================================================*/
{
	/* loop splitting */
	double a,max_aspect, temp_aspect, tempv1[3], tempv2[3], tempv3[3], tempv4[3];
	int j, k, max_aspect_index, pair_central_vertex,
		recurse_central_vertex[MAXPTRS], rc_count;
	struct MC_triangle *pair_triangle, *recurse_triangles[MAXPTRS], *triangle;
	struct MC_vertex *temp_vertices[3], *vertex1, *vertex2;

	/* SAB Could do better with the ordered triangles (keep pointers rather than
		integer indexes or sort the triangles)*/

	/* retriangulate */
	/* 1:  make loop of vertices */
	/* 2: step through and find maximum aspect ratio for split line */
	max_aspect_index=0;
	max_aspect=0;

	if (n_triangles<3)
	{
		display_message(ERROR_MESSAGE,
			"retriangluate.  Less than three triangles!");
		return;
	}

	if (n_triangles==3)
	{
		/* finished - just remove central vertex */
		/* printf("retriangulate: removing central vertex %d\n",i); */

		triangle = triangles[ordered_triangles[0]];
		new_triangles[*new_triangle_count] = triangle;
		(*new_triangle_count)++;
		*discarded_triangle_count = 2;
		discarded_triangles[0] = triangles[ordered_triangles[1]];
		discarded_triangles[1] = triangles[ordered_triangles[2]];

		/* SAB Here I am just replacing the old triangle vertices with the
			new ones without adjusting the Cell, the texture_coord,
			materials, env_map etc. */
		triangle->vertices[0] =
			triangle->vertices[(central_vertex[ordered_triangles[0]]+1) % 3];
		triangle->vertices[1] =
			discarded_triangles[0]->vertices[(central_vertex[ordered_triangles[1]]+1) % 3];
		triangle->vertices[2] =
			discarded_triangles[1]->vertices[(central_vertex[ordered_triangles[2]]+1) % 3];

		remove_triangle_from_vertex_ptrlist( triangle->vertices[0],
			discarded_triangles[1] );
		remove_triangle_from_vertex_ptrlist( triangle->vertices[1],
			discarded_triangles[0] );
		remove_triangle_from_vertex_ptrlist( triangle->vertices[2],
			discarded_triangles[0] );
		remove_triangle_from_vertex_ptrlist( triangle->vertices[2],
			discarded_triangles[1] );
		add_triangle_to_vertex_ptrlist( triangle->vertices[2],
			triangle );

		/* debug
			As the vertex is about to be deleted hopefully it would have no effect
			if these pointers were not updated.
			Check that only the three final triangles belong to the vertex */
		remove_triangle_from_vertex_ptrlist( i_vertex, triangle );
		remove_triangle_from_vertex_ptrlist( i_vertex, discarded_triangles[0] );
		remove_triangle_from_vertex_ptrlist( i_vertex, discarded_triangles[1] );
		if ( i_vertex->n_triangle_ptrs != 0 )
		{
			display_message(ERROR_MESSAGE, "central vertex still has associated triangles!");
		}

		triangle->vertex_index[0] =
			triangle->vertices[0]->vertex_index;
		triangle->vertex_index[1] =
			triangle->vertices[1]->vertex_index;
		triangle->vertex_index[2] =
			triangle->vertices[2]->vertex_index;

		return;
	}

	for ( j = 0 ; j < n_triangles ; j++ )
	{

		vertex1 = triangles[ordered_triangles[j]]->vertices
			[(central_vertex[ordered_triangles[j]]+1) % 3];
		if ( j == n_triangles-1)
		{
			vertex2 = triangles[ordered_triangles[0]]->vertices
				[(central_vertex[ordered_triangles[0]]+2) % 3];
		}
		else
		{
			vertex2 = triangles[ordered_triangles[j+1]]->vertices
				[(central_vertex[ordered_triangles[j+1]]+2) % 3];
		}

		/* calculate aspect ratio by dividing distance of vertice from line with edge length */
		for ( k=0 ; k<3 ; k++)
		{
			tempv1[k] = i_vertex->coord[k] - vertex1->coord[k];
			tempv2[k] = i_vertex->coord[k] - vertex2->coord[k];
			tempv3[k] = vertex2->coord[k] - vertex1->coord[k];
		}
		a = dot_product(tempv1,tempv2) / (vector_modulus(tempv3) * vector_modulus(tempv3));

		for (k=0;k<3;k++)
		{
			tempv4[k] = tempv1[k] + a*(tempv2[k] - tempv1[k]) - i_vertex->coord[k];
		}
		temp_aspect=vector_modulus(tempv4) / vector_modulus(tempv3);
			/* printf("aspect[%d]=%lf\n",j,temp_aspect); */
		if (temp_aspect > max_aspect)
		{
			max_aspect=temp_aspect;
			max_aspect_index=j;
		}
	}

	triangle = triangles[ordered_triangles[max_aspect_index]];
	if ( max_aspect_index == n_triangles-1 )
	{
		pair_triangle = triangles[ordered_triangles[0]];
		pair_central_vertex = central_vertex[ordered_triangles[0]];
	}
	else
	{
		pair_triangle = triangles[ordered_triangles[max_aspect_index+1]];
		pair_central_vertex = central_vertex[ordered_triangles[max_aspect_index+1]];
	}

	new_triangles[*new_triangle_count] = triangle;
	(*new_triangle_count)++;

	temp_vertices[0] =
		triangle->vertices[(central_vertex[ordered_triangles[max_aspect_index]]+1) % 3];
	temp_vertices[1] =
			triangle->vertices[(central_vertex[ordered_triangles[max_aspect_index]]+2) % 3];
	temp_vertices[2] =
		pair_triangle->vertices[(pair_central_vertex+2) % 3];

	add_triangle_to_vertex_ptrlist( temp_vertices[2], triangle );
	remove_triangle_from_vertex_ptrlist( i_vertex, triangle );

	/* now replace ordered triangle list triangle pair with new triangle and recurse */
	/* eliminate last clockwise triangle and move one vertex */

	pair_triangle->vertices
		[(pair_central_vertex+1) % 3] =
		triangle->vertices
		[(central_vertex[ordered_triangles[max_aspect_index]]+1) % 3];
	pair_triangle->vertex_index
		[(pair_central_vertex+1) % 3] =
		pair_triangle->vertices
		[(pair_central_vertex+1) % 3]->vertex_index;
	add_triangle_to_vertex_ptrlist( temp_vertices[0], pair_triangle );
	remove_triangle_from_vertex_ptrlist( temp_vertices[1], pair_triangle );

	triangle->vertices[0] = temp_vertices[0];
	triangle->vertices[1] = temp_vertices[1];
	triangle->vertices[2] = temp_vertices[2];
	triangle->vertex_index[0] = triangle->vertices[0]->vertex_index;
	triangle->vertex_index[1] = triangle->vertices[1]->vertex_index;
	triangle->vertex_index[2] = triangle->vertices[2]->vertex_index;

	/* debug */
	/*	printf ("triangle to keep %d\n", triangle->triangle_index );
	printf("vertices\n");
	for ( j = 0 ; j < 3 ; j++ )
	{
		printf("  %d  >  ", triangle->vertices[j]->vertex_index );
		for (k=0; k <triangle->vertices[j]->n_triangle_ptrs ; k++)
		{
			printf ("%d ", triangle->vertices[j]->triangle_ptrs[k]->triangle_index );
		}
		printf("\n");
	}
	printf ("triangle still to retriangulate %d\n", pair_triangle->triangle_index  );
	printf("vertices\n");
	for ( j = 0 ; j < 3 ; j++ )
	{
		printf("  %d  >  ", pair_triangle->vertices[j]->vertex_index);
		for (k=0; k <pair_triangle->vertices[j]->n_triangle_ptrs ; k++)
		{
			printf ("%d ", pair_triangle->vertices[j]->triangle_ptrs[k]->triangle_index );
		}
		printf("\n");
	}
	*/

	/* reconstruct list for recursion leaving out max_aspect_index triangle
		which is now part of the new triangle list */
	rc_count=0;
	for (j=0;j<n_triangles;j++)
	{
		if (j != max_aspect_index)
		{
			recurse_triangles[rc_count] = triangles[ordered_triangles[j]];
			recurse_central_vertex[rc_count] = central_vertex[ordered_triangles[j]];
			rc_count++;
		}
	}

	for (j=0;j<rc_count;j++)
	{
		triangles[j]=recurse_triangles[j];
		central_vertex[j]=recurse_central_vertex[j];
	}

	/* only had to order once so now store again */
	for (j=0;j<n_triangles /* -1 */;j++)
	{
		ordered_triangles[j]=j;
	}

	retriangulate(iso_surface,i,n_triangles-1, i_vertex, triangles,
		ordered_triangles,central_vertex, new_triangles, new_triangle_count,
		discarded_triangles, discarded_triangle_count);

} /* retriangulate */

#if defined (OLD_CODE)
void retriangulate(struct MC_iso_surface *iso_surface,int i,int n_triangles,
	int triangles[MAXPTRS][3],int ordered_triangles[MAXPTRS],
	int central_vertex[MAXPTRS],int new_triangles[MAXPTRS][3],
	int *new_triangle_ct)
/*******************************************************************************
LAST MODIFIED : 21 December 1995

DESCRIPTION :
Retriangulates hole, removing vertex.
==============================================================================*/
{
	/* loop splitting */
	double tempv1[3],tempv2[3],tempv3[3],tempv4[3],a,max_aspect,temp_aspect;
	int j,k,max_aspect_index,vertex1,vertex2,eliminated_triangle,rc_count;
	int recurse_triangles[MAXPTRS][3],recurse_central_vertex[MAXPTRS];

	/* retriangulate */
	/* 1:  make loop of vertices */
	/* 2: step through and find maximum aspect ratio for split line */
	max_aspect_index=0;
	max_aspect=0;

printf("  enter retriangulate - Not re-implemented yet\n");
#ifdef REDO
/*
for (j=0;j<n_triangles;j++)
{
	for (k=0;k<3;k++)
	{
		printf("%d ",triangles[j][k]);
	}
	printf("\n");
}
*/
	if (n_triangles==3)
	{
		/* finished - just remove central vertex */
/*    printf("retriangulate: removing central vertex %d\n",i); */

		new_triangles[*new_triangle_ct][0]=triangles[ordered_triangles[0]]
															[(central_vertex[ordered_triangles[0]]+1) % 3];
		new_triangles[*new_triangle_ct][1]=triangles[ordered_triangles[1]]
															[(central_vertex[ordered_triangles[1]]+1) % 3];
		new_triangles[*new_triangle_ct][2]=triangles[ordered_triangles[2]]
															[(central_vertex[ordered_triangles[2]]+1) % 3];
		(*new_triangle_ct)++;

		return;
	}

	for (j=0;j<n_triangles;j++) /*????? is this right - i need a break */
	{

		vertex1=triangles[ordered_triangles[j]][(central_vertex[ordered_triangles[j]]+1) % 3];
		if (j==n_triangles-1)
		{
					vertex2=triangles[ordered_triangles[0]][(central_vertex[ordered_triangles[0]]+2) % 3];
		}
		else
		{
					vertex2=triangles[ordered_triangles[j+1]][(central_vertex[ordered_triangles[j+1]]+2) % 3];
		}

		/* calculate aspect ratio by dividing distance of vertice from line with edge length */
		for (k=0;k<3;k++)
		{
			tempv1[k]=iso_surface->vertex_list[i].coord[k] - iso_surface->vertex_list[vertex1].coord[k];
			tempv2[k]=iso_surface->vertex_list[i].coord[k] - iso_surface->vertex_list[vertex2].coord[k];
			tempv3[k]=iso_surface->vertex_list[vertex2].coord[k] - iso_surface->vertex_list[vertex1].coord[k];
		}
		a=dot_product(tempv1,tempv2) / (vector_modulus(tempv3) * vector_modulus(tempv3));

		for (k=0;k<3;k++)
		{
			tempv4[k]=tempv1[k] + a*(tempv2[k] - tempv1[k]) - iso_surface->vertex_list[i].coord[k];
		}
		temp_aspect=vector_modulus(tempv4) / vector_modulus(tempv3);
/*    printf("aspect[%d]=%lf\n",j,temp_aspect); */
		if (temp_aspect > max_aspect)
		{
			max_aspect=temp_aspect;
			max_aspect_index=j;
		}

	}    /* j */
/*
printf("  before new triangles\n");
for (j=0;j<n_triangles;j++)
{
	for (k=0;k<3;k++)
	{
		printf("%d ",triangles[j][k]);
	}
	printf("\n");
}

	printf("-------> max_aspect_index=%d <ordered: %d>\n",max_aspect_index, ordered_triangles[max_aspect_index]);
*/
	/* redo this bit so that all new triangles are stored in new_triangle list and all discarded triangles are stored in
	discarded triangle list */

	/* output new_triangle */

	new_triangles[*new_triangle_ct][0]=triangles[ordered_triangles[max_aspect_index]]
															[(central_vertex[ordered_triangles[max_aspect_index]]+1) % 3];
	new_triangles[*new_triangle_ct][1]=triangles[ordered_triangles[max_aspect_index]]
															[(central_vertex[ordered_triangles[max_aspect_index]]+2) % 3];

	if (max_aspect_index==n_triangles-1)
	{
		new_triangles[*new_triangle_ct][2]=triangles[ordered_triangles[0]][(central_vertex[ordered_triangles[0]]+2) % 3];
	}
	else
	{
		new_triangles[*new_triangle_ct][2]=triangles[ordered_triangles[max_aspect_index+1]]
															[(central_vertex[ordered_triangles[max_aspect_index+1]]+2) % 3];
	}
	(*new_triangle_ct)++;


	/* now replace ordered triangle list triangle pair with new triangle and recurse */
	/* eliminate last clockwise triangle and move one vertice */
	if (max_aspect_index==n_triangles-1)
	{
		triangles[ordered_triangles[max_aspect_index]]
															[(central_vertex[ordered_triangles[max_aspect_index]]+2) % 3]=

					triangles[ordered_triangles[0]]
															[(central_vertex[ordered_triangles[0]]+2) % 3];

		eliminated_triangle=0;
	}
	else
	{
		triangles[ordered_triangles[max_aspect_index]]
															[(central_vertex[ordered_triangles[max_aspect_index]]+2) % 3]=
			/*
					triangles[ordered_triangles[max_aspect_index]+1]
															[(central_vertex[ordered_triangles[max_aspect_index]+1]+2) % 3];
				*/
									triangles[ordered_triangles[max_aspect_index+1]]
															[(central_vertex[ordered_triangles[max_aspect_index+1]]+2) % 3];



		eliminated_triangle=max_aspect_index+1;
	}


	/* reconstruct list for recursion */
	rc_count=0;
	for (j=0;j<n_triangles;j++)
	{
		if (j !=eliminated_triangle)
		{
			for (k=0;k<3;k++)
			{
				recurse_triangles[rc_count][k]=triangles[ordered_triangles[j]][k];

			}
			recurse_central_vertex[rc_count]=central_vertex[ordered_triangles[j]];
			rc_count++;
		}
	}

	for (j=0;j<rc_count;j++)
	{
		for (k=0;k<3;k++)
		{
			triangles[j][k]=recurse_triangles[j][k];
		}
		central_vertex[j]=recurse_central_vertex[j];
	}


	/* only had to order once so now store again */
	for (j=0;j<n_triangles /* -1 */;j++)
	{
		ordered_triangles[j]=j;
	}


	retriangulate(iso_surface,i,n_triangles-1, triangles,ordered_triangles,central_vertex, new_triangles,new_triangle_ct);
#endif
} /* retriangulate */
#endif /* defined (OLD_CODE) */

static void decimate_triangles(struct MC_iso_surface *iso_surface,
	double threshold_distance,int decimation_level)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Decimates triangle mesh
==============================================================================*/
{
	int central_vertex[MAXPTRS], counter, found, i, initial_edge,
		discarded_triangle_count, j, k,n_triangles,
		new_vertex_index, new_triangle_count, new_triangle_index,
		ordered_triangles[MAXPTRS], simple_vertex,
		test_edge, z;
	struct MC_triangle *discarded_triangles[MAXPTRS], *new_triangles[MAXPTRS],
		**new_triangle_position, **old_triangle_position, **temp_triangle_ptrs,
		*triangle, *triangles[MAXPTRS];
	struct MC_vertex *i_vertex, **new_vertex_position, **old_vertex_position;

	double cross[3],magnitude,total_area,vertex_distance,
		v1[3],v2[3],x0[3],x1[3],x2[3],na[3],xc[3],vx[3];

	/* the decimation criteria  - dependent on size of model I guess - will have to send as a parameter */
	/*
		double threshold_distance=0.005;
		int decimation_level=4;
		*/

	for (z=0;z<decimation_level;z++)
	{
		/************** Loop through all vertices in isosurface *************/
		for (i=0;i< iso_surface->n_vertices ;i++)
		{
			i_vertex = iso_surface->compiled_vertex_list[i];
			/**********   step one - classify vertices    **********/
			/* Gather and order triangles around vertex[i] */
			n_triangles = i_vertex->n_triangle_ptrs;

			/* printf("n_triangles=%d\n",n_triangles); */
			counter=0;
			/* get triangles associated with vertices */
			/* triangle_index_count = n_triangles; */

			for (j=0;j<n_triangles;j++)
			{
				central_vertex[j]=4;
				triangle = i_vertex->triangle_ptrs[j];
				triangles[j] = triangle;
				for (k=0;k<3;k++)
				{
					/* debug */
					/* printf("For vertex %d triangles[triangle %d][vertex %d]<triangle index %d> = vertex index %d\n",
						i, j, k, i_vertex->triangle_ptrs[j]->triangle_index,
						triangles[j]->vertices[k]->vertex_index); */

					/* find triangle index for central vertex */
					if (triangles[j]->vertices[k]->vertex_index == i)
					{
						central_vertex[j] = k;
					}
				}
				if ( central_vertex[j] == 4 )
				{
					display_message(ERROR_MESSAGE, "decimate_triangles.  Central vertex not found on triangle in i_vertex list");
					printf("decimate_triangles.  Central vertex not found on triangle in i_vertex list\n");
				}
			}
			/* look for simple classification */
			/* look for clockwise triangle sharing edge (all edges examined share
				the central_vertex */

			simple_vertex=0;
			initial_edge=triangles[0]->vertices[(central_vertex[0]+1) % 3]->vertex_index;
			ordered_triangles[counter]=0;
			counter++;
			test_edge=triangles[0]->vertices[(central_vertex[0]+2) % 3]->vertex_index;
			found=1;

			while (found && !simple_vertex && counter < n_triangles)
			{
				found=0;

				for ( j=0 ; j<n_triangles && !found ; j++)
				{
					if (test_edge==triangles[j]->vertices[(central_vertex[j]+1)%3]->vertex_index)
					{
						found=1;

						test_edge=triangles[j]->vertices[(central_vertex[j]+2)%3]->vertex_index;

						ordered_triangles[counter] = j;
						counter++;
					}
				}
				if (test_edge==initial_edge)
				{
					if (counter != n_triangles)
					{
						display_message(ERROR_MESSAGE, "decimate_triangles.  n_triangles is %d but only %d triangles in sequence", n_triangles, counter );
						printf("Error: counter %d : n_triangles %d\n",counter,n_triangles);
						simple_vertex=0;
						/* Set found = 0 to break loop */
						found = 0;
					}
					else
					{
						simple_vertex=1;
						/*  printf("simple vertex %d\n",i); */
					}
				}
			}

				/*	i_vertex->class=simple_vertex;*/
			i_vertex->vt_class=simple_vertex;

				/* step 2: evaluate decimation criteria */
				/* examine simple vertex  for feature edges here (not implemented yet) */
			if ( simple_vertex )
			{
				na[0] = 0;
				na[1] = 0;
				na[2] = 0;
				xc[0] = 0;
				xc[1] = 0;
				xc[2] = 0;
				total_area = 0;

				/* debug */
				/* printf("n_triangles=%d\n", n_triangles);
				for (j=0;j<n_triangles;j++)
				{
					printf("ordered_triangles[%d]=%d\n",j,ordered_triangles[j]);
				}
				*/

				for (j=0;j<n_triangles;j++)
				{
					/* area */
					for (k=0;k<3;k++)
					{
						v1[k] = triangles[j]->vertices[0]->coord[k] -
							triangles[j]->vertices[1]->coord[k];
						v2[k] = triangles[j]->vertices[0]->coord[k] -
							triangles[j]->vertices[2]->coord[k];

						x0[k] = triangles[j]->vertices[0]->coord[k];
						x1[k] = triangles[j]->vertices[1]->coord[k];
						x2[k] = triangles[j]->vertices[2]->coord[k];
					}
					cross_product(v1,v2,cross);
					magnitude=vector_modulus(cross);

					for (k=0;k<3;k++)
					{
						na[k] += cross[k];
						xc[k] += magnitude * (x0[k] + x1[k] + x2[k]) / 3.0;
					}
					total_area += magnitude;
				}
				for (k=0;k<3;k++)
				{
					na[k] /= total_area;
					xc[k] /= total_area;
					vx[k] = i_vertex->coord[k] - xc[k];
				}
				magnitude = vector_modulus(na);
				for ( k=0 ; k<3 ; k++ )
				{
					na[k] /= magnitude;
				}
				vertex_distance = fabs( dot_product(na,vx) );

				/* debug */
				/*  printf("vertex_distance[%d]=%lf\n",i,vertex_distance); */

				new_triangle_count = 0;
				discarded_triangle_count = 0;

				if (vertex_distance < threshold_distance)
				{
					retriangulate(iso_surface, i, n_triangles, i_vertex,
										triangles, ordered_triangles, central_vertex,
										new_triangles, &new_triangle_count,
										discarded_triangles, &discarded_triangle_count);

					for ( j = 0 ; j < discarded_triangle_count ; j++ )
					{
						discarded_triangles[j]->triangle_index = -1;
					}

					i_vertex->vertex_index = -1;

				}
			} /* if simple_vertex */
		} /* i */

		/*--------------- clean out and renumber vertex and triangle lists ------------------*/

		/* renumber vertices and delete triangle ptrs for reconstruction */
		new_vertex_position = iso_surface->compiled_vertex_list;
		new_vertex_index = 0;
		old_vertex_position = iso_surface->compiled_vertex_list;
		for ( i = 0 ; i < iso_surface->n_vertices ; i++ )
		{
			if ( (*old_vertex_position)->vertex_index != -1 )
			{
				/* Keep this vertex */
				*new_vertex_position = *old_vertex_position;

				/* Update vertex index */
				(*new_vertex_position)->vertex_index = new_vertex_index;

				new_vertex_position++;
				new_vertex_index++;
			}
			else
			{
				/* Free the memory for this vertex */
				if ( (*old_vertex_position)->triangle_ptrs )
				{
					DEALLOCATE ( (*old_vertex_position)->triangle_ptrs );
				}
				DEALLOCATE ( *old_vertex_position );
			}
			old_vertex_position++;
		}


		/* renumber triangles, update vertex_index array and then add this
		triangle to its triangle_ptrs of each of its vertices */

		new_triangle_position = iso_surface->compiled_triangle_list;
		new_triangle_index = 0;
		old_triangle_position = iso_surface->compiled_triangle_list;
		for ( i = 0 ; i < iso_surface->n_triangles ; i++ )
		{
			if ( (*old_triangle_position)->triangle_index != -1 )
			{
				/* Keep this triangle */
				*new_triangle_position = *old_triangle_position;

				/* Update the vertex index from the vertex pointers */
				(*new_triangle_position)->vertex_index[0] =
					(*new_triangle_position)->vertices[0]->vertex_index;
				(*new_triangle_position)->vertex_index[1] =
					(*new_triangle_position)->vertices[1]->vertex_index;
				(*new_triangle_position)->vertex_index[2] =
					(*new_triangle_position)->vertices[2]->vertex_index;

				/* Update triangle index */
				(*new_triangle_position)->triangle_index = new_triangle_index;

				new_triangle_position++;
				new_triangle_index++;
			}
			else
			{
				/* Remove from cell list */
				found = 0;
				j = 0;
				n_triangles = (*old_triangle_position)->cell_ptr->n_triangles
					[(*old_triangle_position)->triangle_list_index];
				temp_triangle_ptrs = (*old_triangle_position)->cell_ptr->triangle_list
					[(*old_triangle_position)->triangle_list_index];
				while ( !found && j < n_triangles )
				{
					if ( *temp_triangle_ptrs == *old_triangle_position )
					{
						found = 1;
						*temp_triangle_ptrs = (struct MC_triangle *)NULL;
					}
					*temp_triangle_ptrs++;
					j++;
				}
				if ( !found )
				{
					display_message(ERROR_MESSAGE,
										"decimate_triangles.   Triangle %x not found in cell list", *old_triangle_position);
				}
				/* Free memory */
				DEALLOCATE ( *old_triangle_position );
			}
			old_triangle_position++;
		}

		/***** !!!!!!!!!! will have to update material pointers etc. !!!!!!!! *****/
		/* printf("Decimation (level %d) : n_vertices (before/after)=%d / %d\n",
				z,iso_surface->n_vertices,new_vertex_index);*/
		/* printf("Triangle decimation: <before/after> :=%d / %d\n",iso_surface->n_triangles,new_triangle_index); */

		iso_surface->n_vertices=new_vertex_index;
		iso_surface->n_triangles=new_triangle_index;
	} /* decimation_level */
} /* decimate_triangles */

#if defined (OLD_CODE)
void decimate_triangles(struct MC_iso_surface *iso_surface,
	double threshold_distance,int decimation_level)
/*******************************************************************************
LAST MODIFIED : 21 December 1995

DESCRIPTION :
Decimates triangle mesh
==============================================================================*/
{
int i,j,k,l,z,found,n_triangles,central_vertex[MAXPTRS],triangles[MAXPTRS][3],
	ordered_triangles[MAXPTRS],new_triangles[MAXPTRS][3],new_triangle_count,
	triangle_index[MAXPTRS];
int test_edge,initial_edge,simple_vertex,temp_count,triangle_to_go,temp_vertex,
	temp_triangle_list[MAXPTRS];
int n_discards, triangle_index_count;
int counter,added_triangle_index;
int new_vertex_index, new_triangle_index;

/* decimation criteria */
double v1[3],v2[3],x0[3],x1[3],x2[3],na[3],xc[3],vx[3],cross[3],magnitude,vertex_distance,total_area;
/* the decimation criteria  - dependent on size of model I guess - will have to send as a parameter */
/*
double threshold_distance=0.005;
int decimation_level=4;
*/

printf("In decimation - Not re-implemented yet\n");
#ifdef REDO
for (z=0;z<decimation_level;z++)
{
/************** Loop through all vertices in isosurface *************/
/* check things havent changed if deletion occurs */

for (i=0;i< iso_surface->n_vertices ;i++)
{


	/* major confusion  over what the hell i am doing with triangle ptrs, arrays etc. - sort it out idiot */
	/**********   step one - classify vertices    **********/
	/* Gather and order triangles around vertex[i] */
	n_triangles=iso_surface->compiled_vertex_list[i]->n_triangle_ptrs;

	/* printf("n_triangles=%d\n",n_triangles); */
	counter=0;
	/* get triangles associated with vertices */
	triangle_index_count=compiled_vertex_list[i]->n_triangle_ptrs;

	for (j=0;j<n_triangles;j++)
	{
		central_vertex[j]=4;
		triangle_index[j]=iso_surface->compiled_vertex_list[i]->triangle_ptrs[j];
		for (k=0;k<3;k++)
		{
/* UP TO HERE IN CHANGING TO MC_ISOSURFACE... */
			triangles[j][k]=iso_surface->triangle_list[iso_surface->
				vertex_list[i].ptrs[j]][k];

			/* debug??? printf("triangles[%d][%d]<%d>=%d [i=%d]\n",j,k,iso_surface->vertex_list[i].ptrs[j],triangles[j][k],i);*/
			/* find triangle index for central vertex */
			if (triangles[j][k]==i)
			{
				central_vertex[j]=k;
			}
		}
		if (central_vertex[j]==4)
		{
			printf("Freakout\n");
		}
	}
	/* look for simple classification */
	/* look for clockwise triangle sharing edge (all edges examined share
	the central_vertex */

	simple_vertex=0;
	initial_edge=triangles[0][(central_vertex[0]+1) % 3];
	ordered_triangles[counter]=0;
	counter++;
	test_edge=triangles[0][(central_vertex[0]+2) % 3];

	found=1;

		do
		{

			found=0;

		for (j=0;j<n_triangles  && !found ;j++)
		{
/*      printf("j=%d:comparing edge %d (%d, %d, %d)\n",j,triangles[j][(central_vertex[j]+1)%3],
			triangles[j][0],
			triangles[j][1],
			triangles[j][2]); */

			if (test_edge==triangles[j][(central_vertex[j]+1)%3])
			{
				found=1;
				/* printf("replacing test_edge %d with %d\n",test_edge,triangles[j][(central_vertex[j]+2)%3]);*/

				test_edge=triangles[j][(central_vertex[j]+2)%3];

				ordered_triangles[counter]=j;
				counter++;
			}
		}
		if (test_edge==initial_edge)
		{
			if (counter !=n_triangles)
			{
				printf("Error: counter %d : n_triangles %d\n",counter,n_triangles);
				simple_vertex=-1;
			}
			else
			{
			simple_vertex=1;
			/*  printf("simple vertex %d\n",i); */
			}
		}
		if (simple_vertex==-1)
		{
			simple_vertex=0;
			break;
		}
	}
	while (found && !simple_vertex);

/* printf("label2\n");  */
	/*???SAB. */
/*	(iso_surface->vertex_list[i]).class=simple_vertex;*/
	(iso_surface->vertex_list[i]).vt_class=simple_vertex;



/* step 2: evaluate decimation criteria */
/* examine simple vertex  for feature edges here (not implemented yet) */
	if (  simple_vertex )
	{
		na[0]=na[1]=na[2]=xc[0]=xc[1]=xc[2]=0;
		total_area=0;

	/*  printf("n_triangles=%d\n", n_triangles); */
/*    for (j=0;j<n_triangles;j++)
		{
			printf("ordered_triangles[%d]=%d\n",j,ordered_triangles[j]);
		}
*/
		for (j=0;j<n_triangles;j++)
		{
			/* area */
			for (k=0;k<3;k++)
			{
				v1[k]=iso_surface->vertex_list[triangles[j][0]].coord[k] -
					iso_surface->vertex_list[triangles[j][1]].coord[k];
				v2[k]=iso_surface->vertex_list[triangles[j][0]].coord[k] -
					iso_surface->vertex_list[triangles[j][2]].coord[k];

				x0[k]=iso_surface->vertex_list[triangles[j][0]].coord[k];
				x1[k]=iso_surface->vertex_list[triangles[j][1]].coord[k];
				x2[k]=iso_surface->vertex_list[triangles[j][2]].coord[k];
			}
			cross_product(v1,v2,cross);
			magnitude=vector_modulus(cross);

			for (k=0;k<3;k++)
			{
				na[k] +=cross[k];
				xc[k] +=magnitude*(x0[k] + x1[k] + x2[k])/3.0;
			}
			total_area +=magnitude;
		}
		for (k=0;k<3;k++)
		{
			na[k] /=total_area;
			xc[k] /=total_area;
			vx[k]=iso_surface->vertex_list[i].coord[k] - xc[k];
		}
		magnitude=vector_modulus(na);
		for (k=0;k<3;k++)
		{
			na[k] /=magnitude;
		}
		vertex_distance=fabs(dot_product(na,vx));
	/*  printf("vertex_distance[%d]=%lf\n",i,vertex_distance); */

		new_triangle_count=0;

	if (vertex_distance < threshold_distance)
		{
			retriangulate(iso_surface, i, n_triangles, triangles,ordered_triangles,central_vertex,new_triangles,&new_triangle_count
			);


		/* now use new triangles and discard triangles to update isosurface triangle and vertice lists */

		/* go through new triangles, and update the vertex pointers to point to the new triangles. Add the new triangles to the list */
		/* add new triangles to list -update vertex pointers */



		/*--------- for each discarded triangle remove references to discarded triangles -----*/

		n_discards=iso_surface->vertex_list[i].n_ptrs;
		for (j=0;j<n_discards;j++)
		{
			triangle_to_go=iso_surface->vertex_list[i].ptrs[j];

			/* now go through each vertex belonging to triangle to go and remove the reference to it */
			for (k=0;k<3;k++)
			{
				temp_vertex=iso_surface->triangle_list[triangle_to_go][k];
				if (temp_vertex !=i)
				{
					temp_count=0;
					/* copy old triangle ptrs to temp_triangle_list if not pointer to discard triangle */
					for (l=0;l<iso_surface->vertex_list[temp_vertex].n_ptrs;l++)
					{
						if ( iso_surface->vertex_list[temp_vertex].ptrs[l] !=triangle_to_go )
						{
							temp_triangle_list[temp_count]=iso_surface->vertex_list[temp_vertex].ptrs[l];
							temp_count++;
						}
						else
						{
						/*  printf("triangle %d removed from vertex list %d\n",triangle_to_go,temp_vertex); */
						}
					}

					/* copy temp list back into main structure */
					for (l=0;l<temp_count;l++)
					{
						iso_surface->vertex_list[temp_vertex].ptrs[l]=temp_triangle_list[l];
					}
					iso_surface->vertex_list[temp_vertex].n_ptrs=temp_count;
				}
			}
		}


		/* ??? nullify all old triangles */
		for (j=0;j<triangle_index_count;j++)
		{
			for(k=0;k<3;k++)
			{
				iso_surface->triangle_list[triangle_index[j]][k]=-1;
			}
		}

		/*--------- add new triangles to isosurface triangle list + update vertex pointers -----*/
		/* as the new number of triangles is always less than the old amount, the old triangle_list spaces can be used */
		added_triangle_index=0;
	/*  printf("new triangle-count=%d\n",new_triangle_count); */
		for (j=0;j < new_triangle_count;j++)
		{
		/*  printf("adding new triangle #%d <%d>\n",j,iso_surface->n_iso_polys); */
			/* inc iso_surf->n_triangles */

			for (k=0;k<3;k++)
			{
				/* add new triangles at end of list */
				iso_surface->triangle_list[triangle_index[added_triangle_index]][k]=new_triangles[j][k];


				/* go to vertices pointed by new triangle and add ptrs to the new triangle */

				iso_surface->vertex_list[new_triangles[j][k]].ptrs[iso_surface->vertex_list[new_triangles[j][k]].n_ptrs]=    triangle_index[added_triangle_index];

				/* increment pointer count for that vertex */
				iso_surface->vertex_list[new_triangles[j][k]].n_ptrs++;
			}
			added_triangle_index++;

		}
	/*---------------------------------------------------------------------------------------*/
		/* make vertex null */
		/* eventually have a pass loop which removes these */
		iso_surface->vertex_list[i].n_ptrs=0;

	} /* if threshold distance */
	} /* if simple vertex */

} /* i */

/*--------------- clean out and renumber vertex and triangle lists ------------------*/

/* go through vertices */
new_vertex_index=0;
i=0;
do
{
	if (iso_surface->vertex_list[i].n_ptrs !=0)
	{

		for (k=0;k<3;k++)
		{
			iso_surface->vertex_list[new_vertex_index].coord[k]=iso_surface->vertex_list[i].coord[k];
			iso_surface->vertex_list[new_vertex_index].normal[k]=iso_surface->vertex_list[i].normal[k];
			iso_surface->vertex_list[new_vertex_index].class=iso_surface->vertex_list[i].class;
		}
		iso_surface->vertex_list[new_vertex_index].n_ptrs=iso_surface->vertex_list[i].n_ptrs;
		for (j=0;j<iso_surface->vertex_list[i].n_ptrs;j++)
		{
			/* copy pointers across */
			iso_surface->vertex_list[new_vertex_index].ptrs[j]=iso_surface->vertex_list[i].ptrs[j];

			for (k=0;k<3;k++)
			{
				/* update triangle list vertex pointers */
				if (iso_surface->triangle_list[iso_surface->vertex_list[new_vertex_index].ptrs[j]][k]==i)
				{
					iso_surface->triangle_list[iso_surface->vertex_list[new_vertex_index].ptrs[j]][k]=new_vertex_index;
				}
			}
		}
		new_vertex_index++;
	}
	i++;
} while (i<iso_surface->n_vertices);

/* clear rest of list */
for (i=new_vertex_index;i<iso_surface->n_vertices;i++)
{
	iso_surface->vertex_list[i].n_ptrs=0;
}

/* go through triangles */
new_triangle_index=0;
i=0;
do
{
	if (iso_surface->triangle_list[i][0] !=-1) /* not a discard */
	{
		for (k=0;k<3;k++)
		{
			iso_surface->triangle_list[new_triangle_index][k]=iso_surface->triangle_list[i][k];
			/* go to vertex list of triangle and update pointers */
			for (j=0;j<iso_surface->vertex_list[iso_surface->triangle_list[new_triangle_index][k]].n_ptrs;j++)
			{
				if (iso_surface->vertex_list[iso_surface->triangle_list[new_triangle_index][k]].ptrs[j]==i)
				{
					iso_surface->vertex_list[iso_surface->triangle_list[new_triangle_index][k]].ptrs[j]=new_triangle_index;
				}
			}
		}
		new_triangle_index++;
	}
	i++;
} while (i<iso_surface->n_iso_polys);


/***** !!!!!!!!!! will have to update material pointers etc. !!!!!!!! *****/
printf("Decimation (level %d) : n_vertices (before/after)=%d / %d\n",
z,iso_surface->n_vertices,new_vertex_index);

printf("Triangle decimation: <before/after> :=%d / %d\n",iso_surface->n_iso_polys,new_triangle_index);
iso_surface->n_vertices=new_vertex_index;
iso_surface->n_iso_polys=new_triangle_index;
} /* decimation_level */
#endif /* REDO */
} /* decimate_triangles */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/
double vector_modulus(double a[3])
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Returns the modulus of vector a.
==============================================================================*/
{
	if (a)
		{
		return( sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]));
		}
	else
		{
		printf(" modulus : invalid vector \n");
		return(1);
		}
} /* vector_modulus */

double dot_product(double a[3],double b[3])
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Returns the dot product ( a.b ) of vectors a,b.
==============================================================================*/
{
	if (a&&b)
		{
		return( a[0]*b[0] + a[1]*b[1] + a[2]*b[2] );
		}
	else
		{
		printf(" dot product : invalid vectors \n");
		return(0);
		}
} /* dot_product */

double scalar_triple_product(double a[3],double b[3],double c[3])
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Returns the scalar triple product ( [abc]=a.(bxc) ) of vectors a,b and c.
==============================================================================*/
{
	if (a&&b&&c)
		{
		return(  a[0] * (b[1]*c[2] - c[1]*b[2])
			+a[1] * (b[2]*c[0] - c[2]*b[0])
			+a[2] * (b[0]*c[1] - b[1]*c[0]) );

		}
	else
		{
		printf(" scalar triple product : invalid vectors \n");
		return(0);
		}
} /* scalar_triple_product */

int normalized_cross_product(float vector_1[3],float vector_2[3],
	float result[3])
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Calculates the normalized cross product of <vector_1> and <vector_2> and puts
it in <result>.
==============================================================================*/
{
	int return_code;
	float norm;

	if (vector_1&&vector_2&&result)
	{
		result[0]=vector_1[1]*vector_2[2] - vector_2[1]*vector_1[2];
		result[1]=vector_1[2]*vector_2[0] - vector_2[2]*vector_1[0];
		result[2]=vector_1[0]*vector_2[1] - vector_1[1]*vector_2[0];
		if ((norm=result[0]*result[0]+result[1]*result[1]+result[2]*result[2])>0)
		{
			norm=(float) sqrt((double)norm);
			result[0] /=norm;
			result[1] /=norm;
			result[2] /=norm;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}

	return (return_code);
} /* normalized_cross_product */

int cross_product(double vector_1[3],double vector_2[3],double result[3])
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Calculates the cross product of <vector_1> and <vector_2> and puts
it in <result>.
==============================================================================*/
{
	int return_code;

	if (vector_1&&vector_2&&result)
		{
		if (vectors_equal(vector_1,vector_2,AREA_ACC))
			{
			if (debug_flag)
				printf("/////////////////// ERROR - cross product not good\n");
			return_code=0;
			}
		else
			{
			result[0]=vector_1[1]*vector_2[2] - vector_2[1]*vector_1[2];
			result[1]=vector_1[2]*vector_2[0] - vector_2[2]*vector_1[0];
			result[2]=vector_1[0]*vector_2[1] - vector_1[1]*vector_2[0];

			return_code=1;
			}
		}
	else
	{
	printf("{{{{{{{{{{{{{{{{ ERROR - ! v1&v2&result\n");
		return_code=0;
	}

	return (return_code);
} /* cross_product */

int vectors_equal(double v1[3],double v2[3],double tolerance)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Compares difference of vectors to a tolerance. returns 1 if less than tolerance,
0 if greater.
==============================================================================*/
{
double v[3];
int i;

for (i=0;i<3;i++)
	{
	v[i]=v1[i]-v2[i];
	}
if (sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]) < tolerance)
	{
	return(1);
	}
else
	{
	return(0);
	}
} /* vectors_equal */

int scalars_equal(double s1,double s2,double tolerance)
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Compares difference of scalars to a tolerance. returns 1 if less than tolerance,
0 if greater.
==============================================================================*/
{

if (fabs(s1-s2) < tolerance)
	{
	return(1);
	}
else
	{
	return(0);
	}
} /* scalars_equal */

#if defined (OLD_CODE)
int load_mc_tables(void)
/*******************************************************************************
LAST MODIFIED : 25 November 1994

DESCRIPTION :
Loads the pntr & data tables
==============================================================================*/
{
	/* the mc case (1-256) */
	int icase;
	int iambig,npolyg;
	int polsiz[4];
	int edgdat[12];
	int index,nalter,nentry,npntr;
	FILE *fp;
	int i,j,k,n,ip;
	char mcubes[100];
	char marchg[100];
	int return_code;

	ENTER(load_mc_tables);
#if defined (FILE_MCUBES) && defined (FILE_MARCHG)
	/* only load tables once if multiple windows opened */
	if (!tablesloaded)
	{
		strcpy(mcubes,FILE_MCUBES);
		strcpy(marchg,FILE_MARCHG);
/*???debug */
printf("%s\n%s\n",mcubes,marchg);
		if (fp=fopen(marchg,"r"))
		{
			npntr=1;
			n=1;
			return_code=1;
			while (return_code&&(n<=256))
			{
				fscanf(fp,"%d",&index);
				if (n==index)
				{
					/* test for case when there are no contours */
					if (index==1 || index==256)
					{
						ipntbl[1 -1][n -1]=0;
					}
					else
					{
						/* see if double contour crossing */
						fscanf(fp,"%d",&iambig);
						if (iambig==0)
						{
							nalter=1;
						}
						else
						{
							nalter=2;
							ipntbl[5 -1][n -1]=iambig;
						}
						/* load up the data arrays with all
						the polygon configurations */
						for (k=1;k<=nalter;k++)
						{
							fscanf(fp,"%d",&npolyg);
							for (i=1;i<=npolyg;i++)
							{
								fscanf(fp,"%d",&polsiz[i -1]);
							}
							nentry=0;
							for (j=1;j<=npolyg;j++)
							{
								nentry +=polsiz[j -1];
							}
							for (i=1;i<=nentry;i++)
							{
								fscanf(fp,"%d",&edgdat[i -1]);
							}
							ip=2*k;
							ipntbl[ip-1 -1][n -1]=npntr;
							ipntbl[ip -1][n -1]=npolyg;
							for (j=0;j<=npolyg-1;j++)
							{
								idatbl[npntr+j -1]=polsiz[j+1 -1];
							}
							for (j=0;j<=nentry-1;j++)
							{
								idatbl[npolyg+npntr+j -1]=edgdat[j+1 -1];
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"load_mc_tables.  Corrupted look up table data");
					return_code=0;
				}
				n++;
			}
			fclose(fp);
			if (return_code)
			{
				/* read triangulated data table */
				if (fp=fopen(mcubes,"r"))
				{
					n=1;
					while (return_code&&(n<=256))
					{
						fscanf(fp,"%d",&icase);
						fscanf(fp,"%d",&npolyg);
						if (icase==n)
						{
							if (icase==1 || icase==256)
							{
								mctbl[1 -1][n -1]=0;
								mctbl[2 -1][n -1]=0;
							}
							else
							{
								iambig=ipntbl[5 -1][n -1];
								mctbl[1 -1][n -1]=iambig;
								mctbl[2 -1][n -1]=npolyg;
								nentry=3*npolyg + 2;
								for (i=3;i<=nentry-1;i++)
								{
									fscanf(fp,"%d,",&mctbl[i -1][n -1]);
								}
								/* (file in unusual format) */
								fscanf(fp,"%d",&mctbl[nentry -1][n -1]);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"load_mc_tables.  icase!=n");
							return_code=0;
						}
						n++;
					}
					fclose(fp);
					if (return_code)
					{
						tablesloaded=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"load_mc_cubes.  Could not open MCUBES");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"load_mc_cubes.  Could not open MARCHG");
			return_code=0;
		}
	}
	else
	{
		return_code=1;
	}
#else
	display_message(ERROR_MESSAGE,"load_mc_cubes.  File names not defined");
	return_code=0;
#endif
	LEAVE;

	return (return_code);
} /* load_mc_cubes */
#endif /* defined (OLD_CODE) */

int marching_cubes(struct VT_scalar_field **scalar_field,int n_scalar_fields,
	struct VT_vector_field *coordinate_field,
	struct MC_iso_surface *mc_iso_surface,
	double *isovalue,int closed_surface,int cutting_plane_on,int decimation)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
The modified marching cubes algorithm for constructing isosurfaces.  This
constructs an isosurface for specified value from a filtered volume texture.
The nodal values are obtained by a weighting of the surrounding cells.  These
are used to create an isosurface, and a polygon list is created.  Normals are
calculated using central or one sided differences at each node.  If
closed_surface, the iso surface generated contains closed surfaces along
intersections with the boundary.
==============================================================================*/
{
	int return_code;
/*???debug */
#if defined (UNIX)
#if defined (DEBUG)
struct tms buffer;
double real_time1,real_time2,real_time3,real_time4,cpu_time1,cpu_time2,
	cpu_time3,cpu_time4;
#endif /* defined (DEBUG) */
/* clock_ticks=100 */
#endif
/* mc array size=2 bigger than standard array nx,ny,nz to account for closed surfaces */
int mcnx, mcny, mcnz;
int x_min, x_max, y_min, y_max, z_min, z_max;
int list_index;
int a_count, a_count2;
int a,b;
int nx,ny,nz;
int i,j,k,m,n,nn,ii;
/* the mc case (1-256) */
int icase[MAX_SCALAR_FIELDS];
int iambig[MAX_SCALAR_FIELDS],npolyg[MAX_SCALAR_FIELDS];
/* size of mesh */

/* pointer to table values */
int ipntr[MAX_SCALAR_FIELDS];
int istep[MAX_SCALAR_FIELDS],iedge[MAX_SCALAR_FIELDS];
/* nodes on either side of contour */
int node1,node2;
/* scalar value at ambiguous node */
double cnrscl;
double sdiff;
double acc=0.000001;
/* normalised contour value */
double p;
/* coords of contour/edge intersection */
double xw,yw,zw;
/* normals */
float rmagf;
/* scalar gradients at nodes */
/*static double dx[8],dy[8],dz[8];*/
/* returned double contour gradient */
double grads;
/* triangle vertex variables */
double v[6][3],v_out[MAX_SCALAR_FIELDS][MAX_INTERSECTION][3][3],
	v_out2[MAX_SCALAR_FIELDS][MAX_INTERSECTION][3][3];
double coord[3];
/* number of triangles after triangle clipped */
int n_triangles[MAX_SCALAR_FIELDS],n_triangles2;
int clip_flag;
/* temp storage for polygons generated by iso surface routine
before reordering as vertex/triangle list */

/* lists specific to border faces */
/*int n_xi_face_polys[6]={0,0,0,0,0,0};*/

/* added detail */
int edge[3],edge_total,ihmax,mm,n_temp_triangles,res;
double h[3],hmax,temp_vout[MAX_INTERSECTION*4][3][3];

#if defined (OLD_CODE)
int *n_polys;
float *polygon_list;
int *cell_index;
int clip_mat[3]={0,0,0};
static int n_clipping_triangles;
#endif /* defined (OLD_CODE) */
struct Triangle **clip_triangles;
struct Triangle *new_triangle;

#if defined (OLD_CODE)
/* start values - iterate through one more layer of cubes if closed surface */
int x0=1,y0=1,z0=1;
int triangle;
int maxp;
int dodgy;
#endif /* defined (OLD_CODE) */
int border_flag=0;
/* variables for calc normals */
float vector1[3],vector2[3],result[3],vectorsum[3],vertex0[3],vertex1[3],
	vertex2[3];

double maxc[3],minc[3],tempc;

/* centre of projection for mapped textures */
#if defined (OLD_CODE)
double cop[3]={0.5,0.5,0.5};
#endif /* defined (OLD_CODE) */
struct MC_cell *mc_cell;
struct MC_triangle *mc_triangle;
float mc_vertices[3][3];

	ENTER(marching_cubes);
#if defined (DEBUG)
	/*???debug */
	printf("enter marching_cubes\n");
#endif /* defined (DEBUG) */
	/* check arguments */
	if (scalar_field&&(*scalar_field)&&(0<n_scalar_fields)&&coordinate_field&&
		mc_iso_surface&&isovalue)
	{
#if defined (DEBUG)
		/*???debug */
		printf("iso_values %d:\n",n_scalar_fields);
		for (i=0;i<n_scalar_fields;i++)
		{
			printf("%d %lg\n",i,isovalue[i]);
		}
#endif /* defined (DEBUG) */
		return_code=1;
#if defined (DEBUG)
#if defined (UNIX)
		/*???debug */
		/* diagnostics */
		real_time1=((double)times(&buffer))/100.0;
		cpu_time1=((double)buffer.tms_utime)/100.0;
		printf("*************  real time=%lf    CPU time=%lf *************\n",
			real_time1,cpu_time1);
#endif /* defined (UNIX) */
#endif /* defined (DEBUG) */
		mc_iso_surface->n_scalar_fields=n_scalar_fields;
#if defined (DEBUG)
		/*???debug */
		printf("1: #MC TRIANGLES=%d  #MC VERTICES=%d\n",mc_iso_surface->n_triangles,
			mc_iso_surface->n_vertices);
#endif /* defined (DEBUG) */
		/* get nx,ny,nz values from field */
		mcnx=mc_iso_surface->dimension[0]+2;
		mcny=mc_iso_surface->dimension[1]+2;
		mcnz=mc_iso_surface->dimension[2]+2;
		nx=scalar_field[0]->dimension[0];
		ny=scalar_field[0]->dimension[1];
		nz=scalar_field[0]->dimension[2];
#if defined (DEBUG)
		/*???debug */
		printf("nx,ny,nz=%d, %d, %d\n",nx,ny,nz);
#endif /* defined (DEBUG) */

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!temp MC */
if (closed_surface)
{
	if (mc_iso_surface->active_block[0]<0)
	{
		mc_iso_surface->active_block[0]=0;
	}
	if (mc_iso_surface->active_block[1]>nx)
	{
		mc_iso_surface->active_block[1]=nx;
	}
	if (mc_iso_surface->active_block[2]<0)
	{
		mc_iso_surface->active_block[2]=0;
	}
	if (mc_iso_surface->active_block[3]>ny)
	{
		mc_iso_surface->active_block[3]=ny;
	}
	if (mc_iso_surface->active_block[4]<0)
	{
		mc_iso_surface->active_block[4]=0;
	}
	if (mc_iso_surface->active_block[5]>nz)
	{
		mc_iso_surface->active_block[5]=nz;
	}
}
else
{
	if (mc_iso_surface->active_block[0]<=0)
	{
		mc_iso_surface->active_block[0]=1;
	}
	if (mc_iso_surface->active_block[1]>nx)
	{
		mc_iso_surface->active_block[1]=nx;
	}
	if (mc_iso_surface->active_block[2]<=0)
	{
		mc_iso_surface->active_block[2]=1;
	}
	if (mc_iso_surface->active_block[3]>ny)
	{
		mc_iso_surface->active_block[3]=ny;
	}
	if (mc_iso_surface->active_block[4]<=0)
	{
		mc_iso_surface->active_block[4]=1;
	}
	if (mc_iso_surface->active_block[5]>nz)
	{
		mc_iso_surface->active_block[5]=nz;
	}
}
/*printf("mc_active_block=(%d, %d), (%d, %d), (%d, %d)\n", mc_iso_surface->active_block[0],
		mc_iso_surface->active_block[1], mc_iso_surface->active_block[2], mc_iso_surface->active_block[3],
		mc_iso_surface->active_block[4], mc_iso_surface->active_block[5]);*/
/*
mc_iso_surface->active_block[0]=1-closed_surface;
mc_iso_surface->active_block[1]=nx+closed_surface;
mc_iso_surface->active_block[2]=1-closed_surface;
mc_iso_surface->active_block[3]=ny+closed_surface;
mc_iso_surface->active_block[4]=1-closed_surface;
mc_iso_surface->active_block[5]=nz+closed_surface;
*/

if (coordinate_field->dimension[0] !=scalar_field[0]->dimension[0]
	|| coordinate_field->dimension[1] !=scalar_field[0]->dimension[1]
	|| coordinate_field->dimension[2] !=scalar_field[0]->dimension[2] )
	{
	printf("ERROR - coordinate field does not match scalar field\n");
	}

if (cutting_plane_on)
	{
	for (i=0;i<n_scalar_fields;i++)
		{
		if (scalar_field[i]->dimension[0] !=scalar_field[0]->dimension[0]
		|| scalar_field[i]->dimension[1] !=scalar_field[0]->dimension[1]
		|| scalar_field[i]->dimension[2] !=scalar_field[0]->dimension[2] )
			{
			printf("ERROR - scalar field[%d] does not match scalar field[0]\n",i);
			}
		}
	}


x_min=mc_iso_surface->active_block[0];
x_max=mc_iso_surface->active_block[1];
y_min=mc_iso_surface->active_block[2];
y_max=mc_iso_surface->active_block[3];
z_min=mc_iso_surface->active_block[4];
z_max=mc_iso_surface->active_block[5];
if (closed_surface)
{
		if (1==x_min)
	x_min=0;
		if (1==y_min)
	y_min=0;
		if (1==z_min)
	z_min=0;
		if (nx==x_max)
	x_max +=1;
		if (ny==y_max)
	y_max +=1;
		if (nz==z_max)
	z_max +=1;
		nx++;
		ny++;
		nz++;
		/*x0=y0=z0=0;*/
}

/* store bounds of cube (for closed surface squishing) */
if (closed_surface)
	{
	rqc(coordinate_field,nx-1,ny-1,nz-1,maxc);
	rqc(coordinate_field,0,0,0,minc);
	/* deal with different +ve axis dirn */
	for (i=0;i<3;i++)
		{
/*???debug */
/*printf("min=%g, max=%g\n",minc[i],maxc[i]);*/
		if (maxc[i] < minc[i])
			{
			tempc=minc[i];
			minc[i]=maxc[i];
			maxc[i]=tempc;
			}
		}
	}

/* delete mc active block first */
for (k=z_min;k<=z_max;k++)
	{
	for (j=y_min;j<=y_max;j++)
		{
		for (i=x_min;i<=x_max;i++)
			{
			/* clear mc_cell triangles and vertices! */
			if (mc_iso_surface->mc_cells !=NULL)
			{
			if (mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny] !=NULL)
					{
					destroy_mc_triangle_list(mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny],
				mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]->triangle_list,
					mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]->n_triangles,  n_scalar_fields, mc_iso_surface);
					mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]->triangle_list=NULL;
					mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]->n_triangles=NULL;
					mc_cell=mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny];
					DEALLOCATE(mc_cell);
					mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]=NULL;
					}
			}
			}
		}
	}
#if defined (DEBUG)
		/*???debug */
		printf("2: #MC TRIANGLES=%d  #MC VERTICES=%d\n",mc_iso_surface->n_triangles,
			mc_iso_surface->n_vertices);
#endif /* defined (DEBUG) */
		if ((mc_iso_surface->n_triangles<0)||(mc_iso_surface->n_vertices<0))
		{
			display_message(ERROR_MESSAGE,"n_triangles or n_vertices < 0");
			mc_iso_surface->n_triangles=0;
			mc_iso_surface->n_vertices=0;
		}
		for (k=z_min;k<=z_max;k++)
		{
			for (j=y_min;j<=y_max;j++)
			{
				for (i=x_min;i<=x_max;i++)
				{
#if defined (DEBUG)
					/*???debug */
					if (i==debug_i && j==debug_j && k==debug_k)
					{
						debug_flag=1;
					}
					else
					{
						debug_flag=0;
					}
					if (debug_flag)
					{
						printf("----x,y,z=%d %d %d ----\n",i,j,k);
					}
#endif /* defined (DEBUG) */
					/* get coords of nodes & scalar values for current cell */
					/* get colours too? */
					n_triangles2=0;
					/* get scalars */
					for (n=0;n<n_scalar_fields;n++)
					{
						fc[n][0]=rqs(scalar_field[n],i-1,j-1,k-1);
						fc[n][1]=rqs(scalar_field[n],i,j-1,k-1);
						fc[n][2]=rqs(scalar_field[n],i-1,j,k-1);
						fc[n][3]=rqs(scalar_field[n],i,j,k-1);
						fc[n][4]=rqs(scalar_field[n],i-1,j-1,k);
						fc[n][5]=rqs(scalar_field[n],i,j-1,k);
						fc[n][6]=rqs(scalar_field[n],i-1,j,k);
						fc[n][7]=rqs(scalar_field[n],i,j,k);
#if defined (DEBUG)
						/*???debug */
						printf("%d %d %d %d  %g %g %g %g %g %g %g %g\n",i,j,k,n,fc[n][0],
							fc[n][1],fc[n][2],fc[n][3],fc[n][4],fc[n][5],fc[n][6],fc[n][7]);
#endif /* defined (DEBUG) */
					}
					/* get coords */
					/* border flag
						0 : part of normal iso surface
						4 : face
						6 : edge
						7 : corner
						8 : 1D in one dirn */
					border_flag=0;
					if (rqc(coordinate_field,i-1,j-1,k-1,coord))
					{
						border_flag++;
					}
					xc[0]=coord[0];
					yc[0]=coord[1];
					zc[0]=coord[2];
					if (rqc(coordinate_field,i,j-1,k-1,coord))
					{
						border_flag++;
					}
					xc[1]=coord[0];
					yc[1]=coord[1];
					zc[1]=coord[2];
					if (rqc(coordinate_field,i-1,j,k-1,coord))
					{
						border_flag++;
					}
					xc[2]=coord[0];
					yc[2]=coord[1];
					zc[2]=coord[2];
					if (rqc(coordinate_field,i,j,k-1,coord))
					{
						border_flag++;
					}
					xc[3]=coord[0];
					yc[3]=coord[1];
					zc[3]=coord[2];
					if (rqc(coordinate_field,i-1,j-1,k,coord))
					{
						border_flag++;
					}
					xc[4]=coord[0];
					yc[4]=coord[1];
					zc[4]=coord[2];
					if (rqc(coordinate_field,i,j-1,k,coord))
					{
						border_flag++;
					}
					xc[5]=coord[0];
					yc[5]=coord[1];
					zc[5]=coord[2];
					if (rqc(coordinate_field,i-1,j,k,coord))
					{
						border_flag++;
					}
					xc[6]=coord[0];
					yc[6]=coord[1];
					zc[6]=coord[2];
					if (rqc(coordinate_field,i,j,k,coord))
					{
						border_flag++;
					}
					xc[7]=coord[0];
					yc[7]=coord[1];
					zc[7]=coord[2];
					/* calculate case number */
					for (m=0;m<n_scalar_fields;m++)
					{
						icase[m]=1;
						for (n=1;n<=8;n++)
						{
							if (fc[m][n -1]>isovalue[m])
							{
								icase[m] +=ibin[n -1];
							}
						}
					}
					/* look thru all clip fields */
					clip_flag=0;
					for (m=0;m<n_scalar_fields;m++)
					{
						if ((icase[m]>1)&&(icase[m]<256))
						{
							clip_flag=1;
						}
					}
					for (m=0;m<n_scalar_fields;m++)
					{
						if (icase[m]==1)
						{
							clip_flag=0;
						}
					}
					/* dont need to consider cases wholly inside or outside surface, or
						wholly clipped */
					/* if [ (not out & not in & not fully clipped) OR (in and partially
						clipped) ] */
					if (clip_flag)
					{
#if defined (DEBUG)
						/*???debug */
						printf("%d %d %d clip_flag\n",i,j,k);
#endif /* defined (DEBUG) */
						/* see if cell has alternative polygon
						configurations */
						for (m=0;m<n_scalar_fields;m++)
						{
							iambig[m]=mctbl[1 -1][icase[m] -1];
							npolyg[m]=mctbl[2 -1][icase[m] -1];
							if (iambig[m]==0)
							{
								/* no double contours */
								/*index[m]=icase[m];*/
								ipntr[m]=3;
								istep[m]=1;
							}
							else
							{
								/*!!! check pointer stuff here */
								/* double contours */
								dblcon(scalar_field[m],nx,ny,nz,i,j,k,iambig[m],isovalue[m],
									&grads);
								cnrscl=fc[m][facnod[iambig[m] -1][1 -1]];
								if (((cnrscl<isovalue[m])&&(grads>0))||
									((cnrscl>isovalue[m])&&(grads<0)))
								{
									ipntr[m]=3;
									istep[m]=1;
								}
								else
								{
									icase[m]=257-icase[m];
									npolyg[m]=mctbl[2 -1][icase[m] -1];
									ipntr[m]=3*npolyg[m] + 2;
									istep[m]=-1;
								}
							}
						}
						/* get the edge indices and perform the interpolation. Iterate
							through all the polygons in the cell */
						n_triangles2=0;
						for (a=0;a<n_scalar_fields;a++)
						{
							n_triangles[a]=0;
							for (m=1;m<=npolyg[a];m++)
							{
								for (n=1;n<=3;n++)  /* x,y,z */
								{
									iedge[a]=mctbl[ipntr[a] -1][icase[a] -1];
									ipntr[a]=ipntr[a]+istep[a];
									node1=nodarr[iedge[a] -1][1 -1];
									node2=nodarr[iedge[a] -1][2 -1];
									sdiff=fc[a][node2 -1]-fc[a][node1 -1];
									if (fabs(sdiff)<acc)
									{
										p=0.5;
									}
									else
									{
										p=(isovalue[a] - fc[a][node1 -1])/sdiff;
									}
									if ((p<0.0)||(p>1.0))
									{
#if defined (DEBUG)
#endif /* defined (DEBUG) */
										/*???debug */
										printf("iso error in case %d\n",icase[a]);
										printf("p=%lf\n",p);
									}
									xw=xc[node1 -1]+p*(xc[node2 -1]-xc[node1 -1]);
									yw=yc[node1 -1]+p*(yc[node2 -1]-yc[node1 -1]);
									zw=zc[node1 -1]+p*(zc[node2 -1]-zc[node1 -1]);
									/* make polygons for rendering */
#if defined (OLD_CODE)
									indptr[a]=6*(n-1);
#endif /* defined (OLD_CODE) */
									v[n-1][0]=xw;
									v[n-1][1]=yw;
									v[n-1][2]=zw;
								} /*n*/
								/*???MS.  Should this go before the above? */
								if ((border_flag!=6)&&(border_flag!=7))
								{
									/* ignore infinitesimal corners & edges as these do not
										contribute to the image, and would stuff up the averages for
										the normals on the faces */
									for (n=0;n<3;n++)
									{
										for (nn=0;nn<3;nn++)
										{
#if defined (OLD_CODE)
											if (0)
											{
												v_out2[a][n_triangles[a]][n][nn]=v[n][nn];
												v_out[a][n_triangles[a]][n][nn]= v[n][nn];
											}
											else
											{
#endif /* defined (OLD_CODE) */
												v_out2[a][n_triangles[a]][2-n][nn]=v[n][nn];
												v_out[a][n_triangles[a]][2-n][nn]= v[n][nn];
#if defined (OLD_CODE)
											}
#endif /* defined (OLD_CODE) */
										}
									}
									/* only store if triangle has area */
									if (vectors_equal(v_out[a][n_triangles[a]][0],
										v_out[a][n_triangles[a]][1],AREA_ACC)||
										vectors_equal(v_out[a][n_triangles[a]][1],
										v_out[a][n_triangles[a]][2],AREA_ACC)||
										vectors_equal(v_out[a][n_triangles[a]][0],
										v_out[a][n_triangles[a]][2],AREA_ACC))
									{
#if defined (DEBUG)
#endif /* defined (DEBUG) */
										if (debug_flag)
										{
											printf("VECTORS EQUAL for triangle\n");
										}
									}
									else
									{
										n_triangles[a]++;
									}
								} /* if border_flag !=6 or 7 */
							} /*m*/
#if !defined (DO_NOT_ADD_DETAIL)
/*???MS.  Detail begin */
/***************************** add detail ************************************/
							if (mc_iso_surface->detail_map)
							{
								for (res=0;res<mc_iso_surface->detail_map[i+j*mcnx+k*mcnx*mcny];
									res++)
								{
									n_temp_triangles=0;
									for (m=0;m<n_triangles[a];m++)
									{
										/* classify edges as internal (internal within cell) or
											boundary internal (internal across cells) */
										for (n=0;n<3;n++)
										{
											for (nn=0;nn<3;nn++)
											{
												v[n][nn]=v_out2[a][m][n][nn];
											}
											edge[n]=0;
										}
										/* compare edges v[0]_v[1],v[1]_v[2],v[2]_v[0] */
										/* 1. check internal */
										/* rotate so that the hypotenuse is always edge 0 */
										h[0]=0;
										h[1]=0;
										h[2]=0;
										hmax=0;
										ihmax=0;
										for (nn=0;nn<3;nn++)
										{
											h[0] += (v[1][nn]-v[0][nn])*(v[1][nn]-v[0][nn]);
											h[1] += (v[2][nn]-v[1][nn])*(v[2][nn]-v[1][nn]);
											h[2] += (v[0][nn]-v[2][nn])*(v[0][nn]-v[2][nn]);
										}
										for (n=0;n<3;n++)
										{
											if (h[n]>hmax)
											{
												hmax=h[n];
												ihmax=n;
											}
										}
										switch (ihmax)
										{
											case 1:
											{
												for (nn=0;nn<3;nn++)
												{
													v[0][nn]=v_out2[a][m][1][nn];
													v[1][nn]=v_out2[a][m][2][nn];
													v[2][nn]=v_out2[a][m][0][nn];
												}
											} break;
											case 2:
											{
												for (nn=0;nn<3;nn++)
												{
													v[0][nn]=v_out2[a][m][2][nn];
													v[1][nn]=v_out2[a][m][0][nn];
													v[2][nn]=v_out2[a][m][1][nn];
												}
											} break;
										}
										for (n=0;n<3;n++)
										{
											for (nn=0;nn<3;nn++)
											{
												v_out2[a][m][n][nn]=v[n][nn];
											}
										}
										for (mm=0;mm<n_triangles[a];mm++)
										{
											if (mm != m)
											{
												if (shared_edge(v[0],v[1],v_out2[a][mm]))
												{
													edge[0]=1;
												}
												if (shared_edge(v[1],v[2],v_out2[a][mm]))
												{
													edge[1]=1;
												}
												if (shared_edge(v[2],v[0],v_out2[a][mm]))
												{
													edge[2]=1;
												}
											}
										}
										/* 2. check iboundary internal */
										if (internal_boundary(v[0],v[1],xc,yc,zc,i,j,k,
											mc_iso_surface->detail_map,mcnx,mcny,mcnz,res))
										{
											edge[0]=1;
										}
										if (internal_boundary(v[1],v[2],xc,yc,zc,i,j,k,
											mc_iso_surface->detail_map,mcnx,mcny,mcnz,res))
										{
											edge[1]=1;
										}
										if (internal_boundary(v[2],v[0],xc,yc,zc,i,j,k,
											mc_iso_surface->detail_map,mcnx,mcny,mcnz,res))
										{
											edge[2]=1;
										}
										/* calculate detail table index */
										edge_total=edge[0]+2*edge[1]+4*edge[2];
										/* calculate new bisected vertices */
										for (nn=0;nn<3;nn++)
										{
											v[3][nn]=(v[0][nn]+v[1][nn])/2.0;
											v[4][nn]=(v[1][nn]+v[2][nn])/2.0;
											v[5][nn]=(v[2][nn]+v[0][nn])/2.0;
										}
										/* replace trinagles using table */
										for (mm=0;mm<detail_table[edge_total][0];mm++)
										{
											for (n=0;n<3;n++)
											{
												for (nn=0;nn<3;nn++)
												{
													temp_vout[n_temp_triangles][n][nn]=
													v[detail_table[edge_total][n+1+mm*3]][nn];
												}
											}
											n_temp_triangles++;
										}
									} /* m */
									for (m=0;m<n_temp_triangles;m++)
									{
										for (n=0;n<3;n++)
										{
											for (nn=0;nn<3;nn++)
											{
												v_out2[a][m][n][nn]=temp_vout[m][n][nn];
											}
										}
									}
									n_triangles[a]=n_temp_triangles;
								} /* detail */
							}
/*???MS.  Detail end */
#endif /* !defined (DO_NOT_ADD_DETAIL) */
						} /* a */
						/* output in poly lists */
						if ((border_flag!=6)&&(border_flag!=7))
						{
							/* ignore infinitesimal corners & edges as these do not contribute
								to the image, and would stuff up the averages for the normals on
								the faces */
							/* we successively clip field a with fields b */
							/* allocate MC_cell triangle list structures */
							/* create mc_cell if not already created */
#if defined (DEBUG)
							/*???debug */
							printf("----------- Allocating MC_cell %d %d %d --------------\n",
								i,j,k);
#endif /* defined (DEBUG) */
							if (mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny] !=NULL)
							{
#if defined (DEBUG)
#endif /* defined (DEBUG) */
								/*???debug */
								printf("had to delete previous entry\n");
								/* deletes cells not deleted if closed surface on */
								destroy_mc_triangle_list(
									mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny],
									mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]->triangle_list,
									mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]->n_triangles,
									n_scalar_fields,mc_iso_surface);
								mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]->triangle_list=
									NULL;
								mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]->n_triangles=
									NULL;
								mc_cell=mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny];
								DEALLOCATE(mc_cell);
								mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]=NULL;
							}
							if (NULL==mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny])
							{
								if (ALLOCATE(mc_cell, struct MC_cell, 1))
								{
									mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]=mc_cell;
								}
								else
								{
									display_message(ERROR_MESSAGE,"ALLOC failed for mc_cell");
									mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]=NULL;
								}
								if (mc_cell)
								{
									mc_cell->index[0]=i;
									mc_cell->index[1]=j;
									mc_cell->index[2]=k;
									/* cell(i,j,k)=mc_cells[i+j*mcnx+k*mcnx*mcny] */
									if (ALLOCATE(mc_cell->triangle_list,struct MC_triangle **,
										n_scalar_fields+6)&&ALLOCATE(mc_cell->n_triangles,int,
										n_scalar_fields+6))
									{
										for (ii=0;ii < n_scalar_fields+6;ii++)
										{
											mc_cell->n_triangles[ii]=0;
											mc_cell->triangle_list[ii]=NULL;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"ALLOC failed for mc_cell triangle list");
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Delete failed for mc_cell");
							}
							/*???SAB.  Trying different ways of clipping */
							/*					for (a=0;a<1;a++)*/
							for (a=0;a<n_scalar_fields;a++)
							{
								n_triangles2=n_triangles[a];
								/* n_triangles[a] represents the polys before clipping,
									n_triangles2 after.  v_out2 contains the triangles after they
									have been clipped by each field */
								for (b=0;b<n_scalar_fields;b++)
								{
									if (a!=b)
									{
										if ((icase[a]>1)&&(icase[a]<256)&&(icase[b]>1)&&
											(icase[b]<256))
										{
											cell_fn.xc=xc;
											cell_fn.yc=yc;
											cell_fn.zc=zc;
											cell_fn.fn=fc[b];
											cell_fn.isovalue=isovalue[b];
											/* 1.0 ???? */        cell_fn.dir= -1.0;
											n_clipped_triangles=0;
											/* have intersecting isosurfaces in cell, so clip */
											/* assemble all clipping triangles in list */
											if ((0<n_triangles[b])&&ALLOCATE(clip_triangles,
												struct Triangle *,n_triangles[b]))
											{
												for (n=0;n<n_triangles[b];n++)
												{
													clip_triangles[n]=make_triangle(v_out[b][n],0,0);
												}
												for (n=0;n<n_triangles2;n++)
												{
													new_triangle=make_triangle(v_out2[a][n],-1,0);
													recurse_clip(new_triangle,n_triangles[b],
														clip_triangles,0);
												}
												for (n=0;n<n_triangles[b];n++)
												{
													if (clip_triangles[n])
													{
														DEALLOCATE(clip_triangles[n]);
													}
												}
												DEALLOCATE(clip_triangles);
											}
											/* outputs n_clipped_triangles into
												clipped_triangle_list */
#if defined (DEBUG)
#endif /* defined (DEBUG) */
											/*???debug */
											if (n_clipped_triangles >=MAX_INTERSECTION)
											{
												printf("#### ERROR n_clipped_triangles=%d ####\n",
													n_clipped_triangles);
											}
											n_triangles2=n_clipped_triangles;
											for (m=0;m<n_triangles2;m++)
											{
												for (n=0;n<3;n++)
												{
													for(nn=0;nn<3;nn++)
													{
														v_out2[a][m][n][nn]=
															clipped_triangle_list[m]->v[n][nn];
													}
												}
												if (clipped_triangle_list[m])
												{
													DEALLOCATE(clipped_triangle_list[m]);
												}
											}
										} /* if icase */
									} /* if a !=b */
								} /* b */
								/* choose which list to use */
								if ((border_flag==0)||(!closed_surface))
								{
									/* standard iso surface */
									list_index=a+6;
								}
								else
								{
									if ((border_flag==4)||(border_flag==8))
									{
										/* face or 1D */
										if (v[0][0]+v[1][0]+v[2][0]<3*minc[0])
										{
											/* x bottom face */
											list_index=0;
										}
										else
										{
											if (v[0][0]+v[1][0]+v[2][0]>3*maxc[0])
											{
												/* x top face */
												list_index=1;
											}
											else
											{
												if (v[0][1]+v[1][1]+v[2][1]<3*minc[1])
												{
													/* y bot */
													list_index=2;
												}
												else
												{
													if(v[0][1]+v[1][1]+v[2][1]>3*maxc[1] )
													{
														/* y top */
														list_index=3;
													}
													else
													{
														if (v[0][2]+v[1][2]+v[2][2]<3*minc[2])
														{
															/* z bot */
															list_index=4;
														}
														else
														{
															if (v[0][2]+v[1][2]+v[2][2]>3*maxc[2])
															{
																/* z top */
																list_index=5;
															}
															else
															{
#if defined (DEBUG)
#endif /* defined (DEBUG) */
																/*???debug */
																printf("****** ERROR : polygon does not lie on face\n");
															}
														}
													}
												}
											}
										}
									}
									else
									{
									printf("########## error border_flag=%d\n",border_flag);
									}
								}
								for (m=0;m<n_triangles2;m++)
								{
									for (n=0;n<3;n++)
									{
										for (nn=0;nn<3;nn++)
										{
											if (closed_surface)
											{
												/* squish to fit inside cube (for closed surface)*/
												if (v_out2[a][m][2-n][nn] > maxc[nn])
												{
													v_out2[a][m][2-n][nn]=maxc[nn];
												}
												if (v_out2[a][m][2-n][nn] < minc[nn])
												{
													v_out2[a][m][2-n][nn]=minc[nn];
												}
											}
											mc_vertices[n][nn]=(float) v_out2[a][m][2-n][nn];
										}
									}
									if (mc_iso_surface->mc_cells[i+j*mcnx+k*mcnx*mcny]==NULL)
									{
#if defined (DEBUG)
#endif /* defined (DEBUG) */
										/*???debug */
										printf("****** ERROR ****** cell (%d %d %d)=NULL\n",i,j,k);
									}
									add_mc_triangle(i,j,k,mc_iso_surface,list_index,mc_vertices,
										x_min,x_max,y_min,y_max,z_min,z_max,mcnx,mcny,mcnz,
										n_scalar_fields+6);
								} /* for m < n_triangles2 */
							} /* a */
						} /* if border flag !=6 or 7*/
					} /*if clip_flag */
				} /*i*/
			} /*j*/
		} /*k*/
#if defined (DEBUG)
#if defined (UNIX)
		/*???debug */
		real_time2=((double)times(&buffer))/100.0;
		cpu_time2= ((double)buffer.tms_utime)/100.0;
#endif
#endif /* defined (DEBUG) */
#if defined (DEBUG)
		/*???debug */
		printf("3: #MC TRIANGLES=%d  #MC VERTICES=%d\n",
			mc_iso_surface->n_triangles,mc_iso_surface->n_vertices);
#endif /* defined (DEBUG) */
		if ((mc_iso_surface->n_triangles>0)&&((return_code=
			compile_mc_vertex_triangle_lists(mc_iso_surface,closed_surface))>0))
		{
			/* create vertex list */
			compile_mc_vertex_triangle_lists(mc_iso_surface, closed_surface);

#if defined (DEBUG)
#if defined (UNIX)
real_time3=((double)times(&buffer))/100.0;
cpu_time3= ((double)buffer.tms_utime)/100.0;
#endif
#endif /* defined (DEBUG) */

#ifdef CHECKLISTS
printf("Checking lists....\n");
for (i=0;i<mc_iso_surface->n_triangles;i++)
{
		if (mc_iso_surface->compiled_triangle_list[i]->triangle_index < 0 ||
	mc_iso_surface->compiled_triangle_list[i]->triangle_index > mc_iso_surface->n_triangles)
	{
			printf ("Index error in triangle_list[%d], triangle_index=%d\n", i,
		mc_iso_surface->compiled_triangle_list[i]->triangle_index);
	}
}
for (i=0;i<mc_iso_surface->n_vertices;i++)
{
		if (mc_iso_surface->compiled_vertex_list[i]->vertex_index < 0 ||
	mc_iso_surface->compiled_vertex_list[i]->vertex_index > mc_iso_surface->n_vertices)
	{
			printf ("Index error in vertex_list[%d], vertex_index=%d\n", i,
		mc_iso_surface->compiled_vertex_list[i]->vertex_index);
	}
}
for (i=0;i<mc_iso_surface->n_triangles;i++)
{
	for(j=0;j<3;j++)
	{
		if (mc_iso_surface->compiled_triangle_list[i]->vertex_index[j] < 0 ||
	mc_iso_surface->compiled_triangle_list[i]->vertex_index[j] > mc_iso_surface->n_vertices)
	{
			printf ("Index error in compiled_triangle_list[%d]->vertex_index[%d]\n", i,j,
		mc_iso_surface->compiled_triangle_list[i]->vertex_index[j]);
	}
		}
}

/*printf("Done.\n");*/
#endif
a_count=0;
for (a=0;a<n_scalar_fields+6;a++)
{
a_count2=0;

		for (i=0;i<mcnx*mcny*mcnz;i++)
		{
		if (mc_iso_surface->mc_cells[i])
	{
	a_count +=mc_iso_surface->mc_cells[i]->n_triangles[a];
	a_count2 +=mc_iso_surface->mc_cells[i]->n_triangles[a];
	}
		}
if (a < 6)
{
		mc_iso_surface->xi_face_poly_index[a]=a_count;
/*    printf("xi_face_poly_index[%d]=%d\n", a, a_count);*/
}

/*if (a>=6)
{
		printf("# Triangles field[%d]=%d\n",a,  a_count2);
}*/
}


		/* Calculate mc normals */
/*???debug */
/*printf("Calculating MC normals....\n");*/
		if(mc_iso_surface->compiled_vertex_list && mc_iso_surface->compiled_triangle_list)
		{
		for (i=0;i<mc_iso_surface->n_vertices;i++)
		{
	for (k=0;k<3;k++)
	{
			vectorsum[k]=0;
	}
	for (j=0;j<mc_iso_surface->compiled_vertex_list[i]->n_triangle_ptrs;j++)
	{
			mc_triangle=mc_iso_surface->compiled_vertex_list[i]->triangle_ptrs[j];
			for (k=0;k<3;k++)
				{
					vertex0[k]=mc_triangle->vertices[0]->coord[k];
					vertex1[k]=mc_triangle->vertices[1]->coord[k];
					vertex2[k]=mc_triangle->vertices[2]->coord[k];

					vector1[k]=vertex1[k]-vertex0[k];
					vector2[k]=vertex2[k]-vertex0[k];
				}
				normalized_cross_product(vector1,vector2,result);
				for (k=0;k<3;k++)
				{
					vectorsum[k] +=result[k];
				}
	}
	/* now set normal as the average & normalize */
	rmagf=(float)sqrt((double)(vectorsum[0]*vectorsum[0]+
			vectorsum[1]*vectorsum[1]+vectorsum[2]*vectorsum[2]));
	for (k=0;k<3;k++)
	{
			mc_iso_surface->compiled_vertex_list[i]->normal[k]=
					-vectorsum[k]/rmagf;
		/*???Mark.  This should be + */
	}
		} /* i */
		}
/*???debug */
/*printf("Done.\n");*/


if (decimation)
	{
	decimate_triangles(mc_iso_surface, 0.01, 3);
	}

#if defined (DEBUG)
#if defined (UNIX)
/*???debug */
real_time4=((double)times(&buffer))/100.0;
cpu_time4= ((double)buffer.tms_utime)/100.0;
/*printf("***************  Isosurface Timing Diagnostics **************\n");
printf("Marching cubes:			Real=%lf    CPU=%lf\n",real_time2-real_time1,cpu_time2-cpu_time1);
printf("Compile Vertex/Triangle lists:	Real=%lf    CPU=%lf\n",real_time3-real_time2,cpu_time3-cpu_time2);
printf("Normal calculation:		Real=%lf    CPU=%lf\n",real_time4-real_time3,cpu_time4-cpu_time3);
printf("_____________________________________________________________\n");
printf("Total time:            Real=%lf    CPU=%lf\n",real_time4-real_time1,cpu_time4-cpu_time1);
printf("Total number of polygons produced: %d\n",mc_iso_surface->n_triangles);
printf("Total number of vertices produced: %d\n",mc_iso_surface->n_vertices);
printf("************************************************************\n\n");*/
/*
timefile=fopen("timefile","a");
fprintf(timefile,"# polys   t_mc  t_vtlist  t_normal  t_total\n");
fprintf(timefile,"%d %lf %lf %lf %lf\n",
iso_surface->n_triangles, cpu_time2-cpu_time1, cpu_time3-cpu_time2, cpu_time4-cpu_time3, cpu_time4-cpu_time1);
fclose(timefile);
*/
#endif /* defined (UNIX) */
#endif /* defined (DEBUG) */

		}
		else
		{
#if defined (DEBUG)
			/*???debug */
			printf("WARNING: No polygons generated \n");
#endif /* defined (DEBUG) */
			mc_iso_surface->compiled_vertex_list=NULL;
			mc_iso_surface->compiled_triangle_list=NULL;
			mc_iso_surface->n_triangles=0;
			mc_iso_surface->n_vertices=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"marching_cubes.  Invalid argument(s)");
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave marching_cubes\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* marching_cubes */

int update_scalars(struct VT_volume_texture *t,double *cutting_plane)
/*******************************************************************************
LAST MODIFIED : 18 October 1997

DESCRIPTION :
Calculates scalar values at nodes from the average of the surrounding cell
values. Also calculates the gradient (using central differences) of the scalar
field. These values are then updated or stored as part of the volume_texture
structure.  1D arrays are used where A[i][j][k]=A'[i + j*dimi + k*dimi*dimj]
???MS.  I think i should put in a provision to edit node values - ie so this
routine cant touch the set nodes.
???DB.  Get rid of MAX_MATERIALS
==============================================================================*/
{
	int return_code;
	int i,j,k,m,nx,ny,nz;
	int count;
	static double dist;
	double val[3],kfactor,sigmadist,sigmadist2;
	int incx,decx,incy,decy,incz,decz;
	struct VT_texture_node *node;
	double clipping,scalar;
	double f;
	struct VT_texture_curve *p;
	double point[3];
	struct Graphical_material *material,*materials[8];
	int l,material_count[8],number_of_distinct_materials;

	ENTER(update_scalars);
	if (t)
	{
		return_code=1;
	nx=t->dimension[0]+1;
	ny=t->dimension[1]+1;
	nz=t->dimension[2]+1;
	kfactor=0.25;
	/* calculate coordinate field */
/* ???debug
printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!B4 update\n");
printVT(t);
*/
	if (t->grid_spacing)
	{
			for (i=0;i<nx;i++)
			{
		val[0]=t->grid_spacing[i];
		for (j=0;j<ny;j++)
			{
			val[1]=t->grid_spacing[j+nx];
			for(k=0;k<nz;k++)
			{
				val[2]=t->grid_spacing[k+nx+ny];
				for (m=0;m<3;m++)
				{
					t->coordinate_field->vector[3*(i+nx*j+nx*ny*k)+m]=
						val[m]*
						(t->ximax[m]-t->ximin[m]) + t->ximin[m];
				}
			}
		}
			}
	}
	else /* regular spacing */
	{
			for (i=0;i<nx;i++)
			{
		val[0]=(double) i;
		for (j=0;j<ny;j++)
		{
			val[1]=(double) j;
			for(k=0;k<nz;k++)
			{
				val[2]=(double) k;
				for (m=0;m<3;m++)
				{
					t->coordinate_field->vector[3*(i+nx*j+nx*ny*k)+m]=
						val[m]/((double) t->dimension[m])*
						(t->ximax[m]-t->ximin[m]) + t->ximin[m];
/*???debug */
/*printf("%g ",t->coordinate_field->vector[3*(i+nx*j+nx*ny*k)+m]);*/
				}
/*???debug */
/*printf("\n");*/
			}
		}
			}
	}
	for (i=0;i<nx;i++)
	{
		for (j=0;j<ny;j++)
		{
			for (k=0;k<nz;k++)
			{
				node=t->global_texture_node_list[ i + j*nx + k*nx*ny ];
				t->scalar_field->scalar[i + j*nx + k*nx*ny]=0;
				/* take average of cells surrounding node */
				/* check to see how many cells surrounding node */
				scalar=0.0;
				count=0;
				incx=incy=incz=decx=decy=decz=0;
				number_of_distinct_materials=0;
				if (i < nx-1) incx=1;
				if (i > 0) decx=1;
				if (j < ny-1) incy=1;
				if (j > 0) decy=1;
				if (k < nz-1) incz=1;
				if (k > 0) decz=1;
				if (incx && incy && incz)
				{
					scalar +=(t->texture_cell_list[
						i + j*(nx-1) +k*(nx-1)*(ny-1)])->scalar_value;
					count++;
#ifdef MAT_CODE
					m=number_of_distinct_materials-1;
					material=
						(t->texture_cell_list[i+j*(nx-1)+k*(nx-1)*(ny-1)])->material;
					while ((m>=0)&&(material!=materials[m]))
					{
						m--;
					}
					if (m>=0)
					{
						material_count[m]++;
					}
					else
					{
						materials[number_of_distinct_materials]=material;
						material_count[number_of_distinct_materials]=1;
						number_of_distinct_materials++;
					}
#endif
				}
				if (decx && decy && decz)
				{
					scalar +=(t->texture_cell_list[
						i-1 + (j-1)*(nx-1) +(k-1)*(nx-1)*(ny-1)])->scalar_value;
					count++;
#ifdef MAT_CODE
					m=number_of_distinct_materials-1;
					material=
						(t->texture_cell_list[i-1+(j-1)*(nx-1)+(k-1)*(nx-1)*(ny-1)])->
						material;
					while ((m>=0)&&(material!=materials[m]))
					{
						m--;
					}
					if (m>=0)
					{
						material_count[m]++;
					}
					else
					{
						materials[number_of_distinct_materials]=material;
						material_count[number_of_distinct_materials]=1;
						number_of_distinct_materials++;
					}
#endif
				}
				if (incx && incy && decz)
				{
					scalar +=(t->texture_cell_list[
						i + j*(nx-1) +(k-1)*(nx-1)*(ny-1)])->scalar_value;
					count++;
#ifdef MAT_CODE
					m=number_of_distinct_materials-1;
					material=(t->texture_cell_list[i+j*(nx-1)+(k-1)*(nx-1)*(ny-1)])->
						material;
					while ((m>=0)&&(material!=materials[m]))
					{
						m--;
					}
					if (m>=0)
					{
						material_count[m]++;
					}
					else
					{
						materials[number_of_distinct_materials]=material;
						material_count[number_of_distinct_materials]=1;
						number_of_distinct_materials++;
					}
#endif
				}
				if (incx && decy && decz)
				{
					scalar +=(t->texture_cell_list[
						i + (j-1)*(nx-1) +(k-1)*(nx-1)*(ny-1)])->scalar_value;
					count++;
#ifdef MAT_CODE
					m=number_of_distinct_materials-1;
					material=
						(t->texture_cell_list[i+(j-1)*(nx-1)+(k-1)*(nx-1)*(ny-1)])->
						material;
					while ((m>=0)&&(material!=materials[m]))
					{
						m--;
					}
					if (m>=0)
					{
						material_count[m]++;
					}
					else
					{
						materials[number_of_distinct_materials]=material;
						material_count[number_of_distinct_materials]=1;
						number_of_distinct_materials++;
					}
#endif
				}
				if (incx && decy && incz)
				{
					scalar +=(t->texture_cell_list[
						i + (j-1)*(nx-1) +k*(nx-1)*(ny-1)])->scalar_value;
					count++;
#ifdef MAT_CODE
					m=number_of_distinct_materials-1;
					material=(t->texture_cell_list[i+(j-1)*(nx-1)+k*(nx-1)*(ny-1)])->
						material;
					while ((m>=0)&&(material!=materials[m]))
					{
						m--;
					}
					if (m>=0)
					{
						material_count[m]++;
					}
					else
					{
						materials[number_of_distinct_materials]=material;
						material_count[number_of_distinct_materials]=1;
						number_of_distinct_materials++;
					}
#endif
				}
				if (decx && decy && incz)
				{
					scalar +=(t->texture_cell_list[
						i-1 + (j-1)*(nx-1) +k*(nx-1)*(ny-1)])->scalar_value;
					count++;
#ifdef MAT_CODE
					m=number_of_distinct_materials-1;
					material=(t->texture_cell_list[i-1+(j-1)*(nx-1)+k*(nx-1)*(ny-1)])->
						material;
					while ((m>=0)&&(material!=materials[m]))
					{
						m--;
					}
					if (m>=0)
					{
						material_count[m]++;
					}
					else
					{
						materials[number_of_distinct_materials]=material;
						material_count[number_of_distinct_materials]=1;
						number_of_distinct_materials++;
					}
#endif
				}
				if (decx && incy && incz)
				{
					scalar +=(t->texture_cell_list[
						i-1 + j*(nx-1) +k*(nx-1)*(ny-1)])->scalar_value;
					count++;
#ifdef MAT_CODE
					m=number_of_distinct_materials-1;
					material=(t->texture_cell_list[i-1+j*(nx-1)+k*(nx-1)*(ny-1)])->
						material;
					while ((m>=0)&&(material!=materials[m]))
					{
						m--;
					}
					if (m>=0)
					{
						material_count[m]++;
					}
					else
					{
						materials[number_of_distinct_materials]=material;
						material_count[number_of_distinct_materials]=1;
						number_of_distinct_materials++;
					}
#endif
				}
				if (decx && incy && decz)
				{
					scalar +=(t->texture_cell_list[
						i-1 + j*(nx-1) + (k-1)*(nx-1)*(ny-1)])->scalar_value;
					count++;
#ifdef MAT_CODE
					m=number_of_distinct_materials-1;
					material=(t->texture_cell_list[i-1+j*(nx-1)+(k-1)*(nx-1)*(ny-1)])->
						material;
					while ((m>=0)&&(material!=materials[m]))
					{
						m--;
					}
					if (m>=0)
					{
						material_count[m]++;
					}
					else
					{
						materials[number_of_distinct_materials]=material;
						material_count[number_of_distinct_materials]=1;
						number_of_distinct_materials++;
					}
#endif
				}
				if (count==0)
				{
					printf("error - count=0\n");
					display_message(ERROR_MESSAGE,"marching cubes: count=0");

					scalar=1.0;
				}
				else
				{
					/*???DB.  Added option for reading in nodal scalars rather than
						calculating from cell values */
					if ((t->calculate_nodal_values)&&!(node->active))
					{
/*            scalar /=(double)count;*/
						scalar /=(double) count;
						node->scalar_value=scalar;
					}
					else
					{
						scalar=node->scalar_value;
						if (t->clipping_field_on)
						{
							clipping=node->clipping_fn_value;
						}
					}
				}
				t->scalar_field->scalar[i + j*nx + k*nx*ny]=scalar;
				/* define a clipping field in the same way as a scalar field */
				if (t->clipping_field_on)
				{
					t->clip_field->scalar[i + j*nx + k*nx*ny]=clipping;
				}
				else
				{
				if (cutting_plane)
				{
/*???MS.  have some fun here */
#if defined (BOOLEAN_TEST)

				f=/* 1.0 */ (cutting_plane[0]*((double)i)/((double)t->dimension[0])+cutting_plane[1]*((double)j)/((double)t->dimension[1])+
					cutting_plane[2]* ((double) k)/((double)t->dimension[2]));
					/*printf("(%d,%d,%d)=%f\n",i,j,k,f);*/
				node->clipping_fn_value=0.5;
				t->clip_field->scalar[i + j*nx + k*nx*ny]=1.0-f;
#else
				/* make scalar value a function : f=ax=by +cz (- d d is iso) */
/*        f=cutting_plane[0]*(double)i+cutting_plane[1]*(double)j+
					cutting_plane[2]* (double) k;*/
				f=/*1.0 - */ 0.1*(cutting_plane[0]* (double) i*i +
					cutting_plane[1]* (double) j*j + cutting_plane[2]* (double) k*k);
				node->clipping_fn_value=f;
				t->clip_field->scalar[i + j*nx + k*nx*ny]=f;
/*???debug */
/*printf("f=%g\n",f);*/
#endif /* defined (BOOLEAN_TEST) */
				}
/*???MS.  end of fun */
				}

#ifdef MAT_CODE
				/* assign dominant non-zero material index to node */
				m=0;
				for (l=number_of_distinct_materials-1;l>0;l--)
				{
					if (material_count[l]>material_count[m])
					{
						m=l;
					}
				}
/*???debug */
/*printf("max=%s\n",(materials[m])->name);*/
				if (node->dominant_material)
				{
					DEACCESS(Graphical_material)(&(node->dominant_material));
				}
				if (node->material)
				{
					node->dominant_material=ACCESS(Graphical_material)(node->material);
				}
				else
				{
					node->dominant_material=ACCESS(Graphical_material)(materials[m]);
				}
#endif

				/* put in potential contribution from texture_curves */
				for (m=0;m<3;m++)
				{
					point[m]=t->coordinate_field->vector[3*(i+nx*j+nx*ny*k)+m] ;
				}
				/* add contribution to point from each line segment */
				sigmadist=sigmadist2=0;
				for (p=*(t->texture_curve_list); p; p=p->ptrnext)
				{
					switch (p->type)
					{
						case 0:
						{
							dist= blob_segment_distance(0.75*kfactor,p->point1,p->point2,
								point,p->scalar_value[0],p->scalar_value[1]);
						} break;
						case 1:
						{
							dist= line_segment_distance(kfactor,p->point1,p->point2,point,
								p->scalar_value[0],
							p->scalar_value[1]);
						} break;
						case 2:
						{
							dist= curve_segment_distance(kfactor,p->point1,p->point3,
								p->point4,p->point2,point,p->scalar_value[0],
								p->scalar_value[1]);

						} break;
						case 3:
						{
							dist=0;
							t->scalar_field->scalar[i + j*nx + k*nx*ny] +=soft_object_distance(p->scalar_value[1],p->point1,
								point,p->scalar_value[0]);
						} break;
					}
					sigmadist +=dist;
					sigmadist2 +=dist*dist;
				}
				/* Vtotal=(Va + Vb + ..)/(Va^2 + Vb^2 + ...) */
				if (*(t->texture_curve_list) !=(struct VT_texture_curve *) NULL)
				{
					if (sigmadist2==0)
					{
						t->scalar_field->scalar[i + j*nx + k*nx*ny] += 0;
					}
					else if (t->scalar_field->scalar[i + j*nx + k*nx*ny] >0.0)
					{
						if (sigmadist/sigmadist2 < 1.0)
						{
							t->scalar_field->scalar[i + j*nx + k*nx*ny] +=
								1.0 - sigmadist/sigmadist2;
						}
						else
						{
							t->scalar_field->scalar[i + j*nx + k*nx*ny] +=0;
						}
					}
					else
					{
						t->scalar_field->scalar[i + j*nx + k*nx*ny] +=
							1.0 - sigmadist/sigmadist2;

					}
				}
				if (node->active)
				{
					/* if node active it dominates all */
					(t->scalar_field->scalar)[i+j*nx+k*nx*ny]=node->scalar_value;
				}
			}
		}
	}
/* debug
printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!after update\n");
printVT(t);
*/
	}
	else
	{
		display_message(ERROR_MESSAGE,"update_scalars.  Invalid argument(s)");
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* update_scalars */

void print_texture(struct VT_volume_texture *texture)
/*******************************************************************************
LAST MODIFIED : 24 November 1994

DESCRIPTION :
Prints volume_texure scalar info into a file
==============================================================================*/
{
FILE *fp;
int n,i,j,k;
int dim[3];
double sv;
struct Graphical_material *material;

fp=fopen("svfile","w");
/* print layers of nodal scalar info */
for (n=0;n<3;n++)
	{
	dim[n]=texture->dimension[n]+1;
	}
for (k=0;k<dim[2];k++)
	{
	for (j=0;j<dim[1];j++)
		{
		for (i=0;i<dim[0];i++)
			{
			sv=rqs(texture->scalar_field,i,j,k);
			fprintf(fp,"%.2f ",sv);
			}
		fprintf(fp,"\n");
		}
	fprintf(fp,"\n");
	}
fprintf(fp,"Material\n");
for (k=0;k<dim[2];k++)
	{
	for (j=0;j<dim[1];j++)
		{
		for (i=0;i<dim[0];i++)
			{
			material=rqm(texture,i,j,k);
			fprintf(fp,"%s ",Graphical_material_name(material));
			}
		fprintf(fp,"\n");
		}
	fprintf(fp,"\n");
	}
fprintf(fp,"Cell Material\n");
for (k=0;k<dim[2]-1;k++)
	{
	for (j=0;j<dim[1]-1;j++)
		{
		for (i=0;i<dim[0]-1;i++)
			{
			material=rqmc(texture,i,j,k);
			fprintf(fp,"%s ",Graphical_material_name(material));
			}
		fprintf(fp,"\n");
		}
	fprintf(fp,"\n");
	}

fprintf(fp,"Cell Scalar\n");
for (k=0;k<dim[2]-1;k++)
	{
	for (j=0;j<dim[1]-1;j++)
		{
		for (i=0;i<dim[0]-1;i++)
			{
			sv=rqsc(texture,i,j,k);
			fprintf(fp,"%.2f ",sv);
			}
		fprintf(fp,"\n");
		}
	fprintf(fp,"\n");
	}

fclose(fp);
} /* print_texture */

void destroy_mc_triangle_list(struct MC_cell *mc_cell,
	struct MC_triangle ***triangle_list,int *n_triangles,int n_scalar_fields,
	struct MC_iso_surface *mc_iso_surface)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Destroy triangle list for a mc_cell, which includes (n_scalar_fields + 6)
independent triangle lists
==============================================================================*/
{
	int i;
	struct MC_triangle **field_list;

	ENTER(destroy_mc_triangle_list);
	if (triangle_list && n_triangles)
	{
		for (i=0;i < n_scalar_fields + 6;i++)
		{
			field_list=triangle_list[i];
			if (field_list&&n_triangles[i])
			{
				destroy_mc_triangle_field_list(mc_cell,field_list,n_triangles[i],
					mc_iso_surface);
			}
		}
		DEALLOCATE(n_triangles);
		DEALLOCATE(triangle_list);
	}
} /* destroy_mc_triangle_list */

#if defined (OLD_CODE)
void test_clip(void)
{
int m,n,nn;
struct Triangle **clip_triangles;
struct Triangle *new_triangle;
/*
double t1[1][3][3]={{ {1,0,0},{0,1,0},{0,0,1} }};
*/
double t1[2][3][3]={{ {1,0,0},{0,1,0},{0,0,1} },
					{ {0,0,0},{1,0,0},{0,0,1}}};

int tm[3]={0,0,0};
/* clip */
/*
double t2[1][3][3]={{ {0.5,0,0},{0,.5,0},{0,0,.5}}};
*/

double t2[2][3][3]={{ {0,0.5,2},{2,0.5,0},{0,0.5,0}},

			{ {0,0.6,0},{2,0.6,0},{0,0.6,2}}};
int n_clipping_triangles=2;
int n_triangles=1;
int clip_mat[3]={0,0,0};

n_clipped_triangles=0;

debug_flag=1;
/*???DB.  Check allocation */
ALLOCATE(clip_triangles,struct Triangle *,n_clipping_triangles);
for (n=0;n<n_clipping_triangles;n++)
		{
		clip_triangles[n]=make_triangle(t2[n],0,0);
		}
for (n=0;n<n_triangles;n++)
		{
		new_triangle=make_triangle(t1[n],-1,0);
		recurse_clip(new_triangle,n_clipping_triangles,clip_triangles,0);
		}
/* outputs n_clipped_triangles into clipped_triangle_list */

for (m=0;m<n_clipped_triangles;m++)
	{
	printf("t[%d] : ",m);
	for (n=0;n<3;n++)
		{
		for(nn=0;nn<3;nn++)
			{
			printf("%.3f ",clipped_triangle_list[m]->v[n][nn]);
			}
		printf(" | ");
		}
	printf("\n");
	}

printf("***********************************************\n");

DEALLOCATE(clip_triangles);

n_clipped_triangles=0;
/*???DB.  Check allocation */
ALLOCATE(clip_triangles,struct Triangle *,n_triangles);
for (n=0;n<n_triangles;n++)
		{
		clip_triangles[n]=make_triangle(t1[n],0,0);
		}
for (n=0;n<n_clipping_triangles;n++)
		{
		new_triangle=make_triangle(t2[n],0,0);
		recurse_clip(new_triangle,n_clipping_triangles,clip_triangles,0);
		}
/* outputs n_clipped_triangles into clipped_triangle_list */



for (m=0;m<n_clipped_triangles;m++)
	{
	printf("t[%d] : ",m);
	for (n=0;n<3;n++)
		{
		for(nn=0;nn<3;nn++)
			{
			printf("%.3f ",clipped_triangle_list[m]->v[n][nn]);
			}
		printf(" | ");
		}
	printf("\n");
	}
} /* test_clip */
#endif /* defined (OLD_CODE) */

int calculate_mc_material(struct VT_volume_texture *texture,
	struct MC_iso_surface *mc_iso_surface)
/*******************************************************************************
LAST MODIFIED : 26 February 1997

DESCRIPTION :
Calculates the material/colour field to accompany the isosurface (ie allocates
colours to the polygons)
==============================================================================*/
{
  int return_code;
  double coord1[3],coord2[3];
  int i,j,k,m,n;
  struct MC_triangle *triangle;
  
  ENTER(calculate_mc_material);
  if (texture&&mc_iso_surface)
  {
		return_code=1;
		for (n=0;n<mc_iso_surface->n_triangles;n++)
		{
			triangle=mc_iso_surface->compiled_triangle_list[n];
#if defined (OLD_CODE)
      if (1/*triangle->cell_ptr*/)
      {
#endif /* defined (OLD_CODE) */
        /* calculate the cell index ( to get the material from ) */
        i=triangle->cell_ptr->index[0];
        j=triangle->cell_ptr->index[1];
        k=triangle->cell_ptr->index[2];
        /* take two opposing nodes */
        rqc(texture->coordinate_field,i-1,j-1,k-1,coord1);
        rqc(texture->coordinate_field,i,j,k,coord2);
        /* take cell material if it exists, else interpolate using nodes */
        if (rqenvc(texture,i-1,j-1,k-1) !=0 )
        {
          for (m=0;m<3;m++)
          {
            triangle->env_map[m]=rqenvc(texture,i-1,j-1,k-1);
            rqmc_cop(&(triangle->iso_poly_cop[m][0]),texture,i-1,j-1,k-1);
          }
        }
        else
        {
          for (m=0;m<3;m++)
          {
            triangle->env_map[m]=(struct Environment_map *) NULL;
            rqmc_cop(&(triangle->iso_poly_cop[m][0]),texture,i-1,j-1,k-1);
          }
        }
        /* take cell material if it exists, else interpolate using nodes */
        if (rqmc(texture,i-1,j-1,k-1) !=0 )
        {
          for (m=0;m<3;m++)
          {
            triangle->material[m]=rqmc(texture,i-1,j-1,k-1);
            rqmc_cop(&(triangle->iso_poly_cop[m][0]),texture,i-1,j-1,k-1);
          }
        }
        else if (!rqenvc(texture,i-1,j-1,k-1))
        {
          /*printf("!!!!!  material + env map do not exist\n");*/
          triangle->material[m]=NULL;
          for (m=0;m<3;m++)
          {
            triangle->iso_poly_cop[m][0]=0;
            triangle->iso_poly_cop[m][1]=0;
            triangle->iso_poly_cop[m][2]=0;
          }
        }
#if defined (OLD_CODE)
      }
#endif /* defined (OLD_CODE) */
		} /* n */
    /* examine  nearest node to the vertex to determine closest material value */
  }
  else
  {
    display_message(ERROR_MESSAGE,"calculate_mc_material.  Invalid argument(s)");
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* calculate_mc_material */

#if defined (OLD_CODE)
/* SAB Seems we can combine these functions and make then work for a
general coordinate rather than VT or MC structures */
void cube_map_function(int *index,float *texturemap_coord,struct VT_iso_vertex *vertex, double cop[3],
	double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 18.10.95
DESCRIPTION :
Calculates u,v texture map values for a vertex. Third texture coordinate set to zero. (anticpation of SGI 3d texture mapping potential combination)

Calculates intersection from a centre of projection through a vertex onto the
surface of a cube - returns u,v coords on surface, and index (1-6) of
intersected surface.
==============================================================================*/
{
int i;
double t,u,v,sf[3];

/* if VT spread over several elements, then stretch texture out over these by
	dividing by xi range */
for (i=0;i<3;i++)
{
	if (ximax[i]-ximin[i] > 1.0)
	{
		sf[i]=ximax[i] - ximin[i];
	}
	else
	{
		sf[i]=1.0;
	}

}


/* check intersect face 5 (z=0) */

t= -(cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);

u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (t < 1) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=4;
	texturemap_coord[0]=(float)1.0 - u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 6 (z=1) */

t= (1.0 -cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);

u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{

	/* valid coordinate */
	*index=5;
	texturemap_coord[0]=(float)u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}


/* check intersect face 1 (x=0) */

t= -(cop[0]/sf[0])/((vertex->coord[0] - cop[0])/sf[0]);
u=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=0;
	texturemap_coord[0]=(float)u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 2 (x=1) */

t= (1.0 -cop[0]/sf[0])/((vertex->coord[0] - cop[0])/sf[0]);
u=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=1;
	texturemap_coord[0]=(float)1.0 - u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 3 (y=0) */

t= -(cop[1]/sf[1])/((vertex->coord[1] - cop[1])/sf[1]);
u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=2;
	texturemap_coord[0]=(float) u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 4 (y=1) */

t= (1.0 -cop[1]/sf[1])/((vertex->coord[1] - cop[1])/sf[1]);
u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=3;
	texturemap_coord[0]=(float)u;
	texturemap_coord[1]=(float)1.0 - v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 5 (z=0) */

t= -(cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);

u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=4;
	texturemap_coord[0]=(float)1.0 - u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 6 (z=1) */

t= (1.0 -cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);

u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{

	/* valid coordinate */
	*index=5;
	texturemap_coord[0]=(float)u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

printf("ERROR: cube_map_function : no valid intersection\n");

}



void face_cube_map_function(int *index,float texturemap_coord[3],
	struct VT_iso_vertex *vertex,double cop[3],int face_index,
	double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 18/10/95
DESCRIPTION :
Calculates u,v texture map values for a vertex on a face.

This varies from the general case in that face values are fixed as the projection is made
onto them.

==============================================================================*/
{
int i;
double t,u,v,sf[3];

/* if VT spread over several elements, then stretch texture out over these by
	dividing by xi range */
for (i=0;i<3;i++)
{
	if (ximax[i]-ximin[i] > 1.0)
	{
		sf[i]=ximax[i] - ximin[i];
	}
	else
	{
		sf[i]=1.0;
	}

}

switch(face_index)
{
	case (1):
		t= -(cop[0]/sf[0])/((vertex->coord[0] - cop[0])/sf[0]);
		u=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
		v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
		*index=0;
		texturemap_coord[0]=(float) u;
		texturemap_coord[1]=(float) v;
		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(2):

		t= (1.0 -cop[0]/sf[0])/((vertex->coord[0] - cop[0])/sf[0]);
		u=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
		v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];

		*index=1;
		texturemap_coord[0]=(float)1.0 - u;
		texturemap_coord[1]=(float)v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(3):

		t= -(cop[1]/sf[1])/((vertex->coord[1] - cop[1])/sf[1]);
		u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
		v=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];

		*index=2;
		texturemap_coord[0]=(float)u;
		texturemap_coord[1]=(float)v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(4):


		t= (1.0 -cop[1]/sf[1])/((vertex->coord[1] - cop[1])/sf[1]);
		u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
		v=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];

		/* valid coordinate */
		*index=3;
		texturemap_coord[0]=(float)u;
		texturemap_coord[1]=(float)1.0 - v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(5):

		t= -(cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);
		u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
		v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];

		*index=4;
		texturemap_coord[0]=(float)1.0 - u;
		texturemap_coord[1]=(float)v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(6):

		t= (1.0 -cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);
		u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
		v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];

		/* valid coordinate */
		*index=5;
		texturemap_coord[0]=(float)u;
		texturemap_coord[1]=(float)v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;
}
}
/*---------------------------------------------*/

void mc_cube_map_function(int *index,float *texturemap_coord,struct MC_vertex *vertex, double cop[3],
	double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 26 February 1997
DESCRIPTION :
Calculates u,v texture map values for a vertex. Third texture coordinate set to zero.
(anticpation of SGI 3d texture mapping potential combination)

Calculates intersection from a centre of projection through a vertex onto the
surface of a cube - returns u,v coords on surface, and index (1-6) of
intersected surface.
==============================================================================*/
{
int i;
double t,u,v,sf[3];

/* if VT spread over several elements, then stretch texture out over these by
	dividing by xi range */
for (i=0;i<3;i++)
{
	if (ximax[i]-ximin[i] > 1.0)
	{
		sf[i]=ximax[i] - ximin[i];
	}
	else
	{
		sf[i]=1.0;
	}

}


/* check intersect face 5 (z=0) */

t= -(cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);

u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (t < 1) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=4;
	texturemap_coord[0]=(float)1.0 - u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 6 (z=1) */

t= (1.0 -cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);

u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{

	/* valid coordinate */
	*index=5;
	texturemap_coord[0]=(float)u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}


/* check intersect face 1 (x=0) */

t= -(cop[0]/sf[0])/((vertex->coord[0] - cop[0])/sf[0]);
u=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=0;
	texturemap_coord[0]=(float)u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 2 (x=1) */

t= (1.0 -cop[0]/sf[0])/((vertex->coord[0] - cop[0])/sf[0]);
u=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=1;
	texturemap_coord[0]=(float)1.0 - u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 3 (y=0) */

t= -(cop[1]/sf[1])/((vertex->coord[1] - cop[1])/sf[1]);
u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=2;
	texturemap_coord[0]=(float) u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 4 (y=1) */

t= (1.0 -cop[1]/sf[1])/((vertex->coord[1] - cop[1])/sf[1]);
u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=3;
	texturemap_coord[0]=(float)u;
	texturemap_coord[1]=(float)1.0 - v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 5 (z=0) */

t= -(cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);

u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{
	/* valid coordinate */
	*index=4;
	texturemap_coord[0]=(float)1.0 - u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

/* check intersect face 6 (z=1) */

t= (1.0 -cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);

u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
{

	/* valid coordinate */
	*index=5;
	texturemap_coord[0]=(float)u;
	texturemap_coord[1]=(float)v;

	/* for present mapped textures are 2d */
	texturemap_coord[2]=0;
	return;
}

printf("ERROR: cube_map_function : no valid intersection\n");

}



void mc_face_cube_map_function(int *index,float texturemap_coord[3],
	struct MC_vertex *vertex,double cop[3],int face_index,
	double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 26 February 1997
DESCRIPTION :
Calculates u,v texture map values for a vertex on a face.

This varies from the general case in that face values are fixed as the projection is made
onto them.

==============================================================================*/
{
int i;
double t,u,v,sf[3];

/* if VT spread over several elements, then stretch texture out over these by
	dividing by xi range */
for (i=0;i<3;i++)
{
	if (ximax[i]-ximin[i] > 1.0)
	{
		sf[i]=ximax[i] - ximin[i];
	}
	else
	{
		sf[i]=1.0;
	}

}

switch(face_index)
{
	case (1):
		t= -(cop[0]/sf[0])/((vertex->coord[0] - cop[0])/sf[0]);
		u=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
		v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];
		*index=0;
		texturemap_coord[0]=(float) u;
		texturemap_coord[1]=(float) v;
		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(2):

		t= (1.0 -cop[0]/sf[0])/((vertex->coord[0] - cop[0])/sf[0]);
		u=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];
		v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];

		*index=1;
		texturemap_coord[0]=(float)1.0 - u;
		texturemap_coord[1]=(float)v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(3):

		t= -(cop[1]/sf[1])/((vertex->coord[1] - cop[1])/sf[1]);
		u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
		v=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];

		*index=2;
		texturemap_coord[0]=(float)u;
		texturemap_coord[1]=(float)v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(4):


		t= (1.0 -cop[1]/sf[1])/((vertex->coord[1] - cop[1])/sf[1]);
		u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
		v=(cop[2] + t*(vertex->coord[2]-cop[2]))/sf[2];

		/* valid coordinate */
		*index=3;
		texturemap_coord[0]=(float)u;
		texturemap_coord[1]=(float)1.0 - v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(5):

		t= -(cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);
		u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
		v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];

		*index=4;
		texturemap_coord[0]=(float)1.0 - u;
		texturemap_coord[1]=(float)v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;

	case(6):

		t= (1.0 -cop[2]/sf[2])/((vertex->coord[2] - cop[2])/sf[2]);
		u=(cop[0] + t*(vertex->coord[0]-cop[0]))/sf[0];
		v=(cop[1] + t*(vertex->coord[1]-cop[1]))/sf[1];

		/* valid coordinate */
		*index=5;
		texturemap_coord[0]=(float)u;
		texturemap_coord[1]=(float)v;

		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
		break;
}
}
#endif /* defined (OLD_CODE) */

void cube_map_function(int *index,float *texturemap_coord,
	struct VT_iso_vertex *vertex,double cop[3],double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
==============================================================================*/
{
	ENTER(cube_map_function);
	general_cube_map_function(index,texturemap_coord,vertex->coord,cop,ximin,
		ximax);
	LEAVE;
} /* cube_map_function */

void face_cube_map_function(int *index,float texturemap_coord[3],
	struct VT_iso_vertex *vertex,double cop[3],int face_index,
	double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
==============================================================================*/
{
	ENTER(face_cube_map_function);
	general_face_cube_map_function(index,texturemap_coord,vertex->coord,cop,
		face_index,ximin,ximax);
	LEAVE;
} /* face_cube_map_function */

void mc_cube_map_function(int *index,float *texturemap_coord,
	struct MC_vertex *vertex,double cop[3],double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
==============================================================================*/
{
	ENTER(mc_cube_map_function);
	general_cube_map_function(index,texturemap_coord,vertex->coord,cop,ximin,
		ximax);
	LEAVE;
} /* mc_cube_map_function */

void mc_face_cube_map_function(int *index,float texturemap_coord[3],
	struct MC_vertex *vertex,double cop[3],int face_index,double ximin[3],
	double ximax[3])
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
==============================================================================*/
{
	ENTER(mc_face_cube_map_function);
	general_face_cube_map_function(index,texturemap_coord,vertex->coord,cop,
		face_index,ximin, ximax);
	LEAVE;
} /* mc_face_cube_map_function */

void general_cube_map_function_hack(int *index,float *texturemap_coord,
	float *vertex_coords,double cop[3],double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Calculates u,v texture map values for a vertex. Third texture coordinate set to
zero (in anticipation of SGI 3d texture mapping potential combination).

Calculates intersection from a centre of projection through a vertex onto the
surface of a cube - returns u,v coords on surface, and index (1-6) of
intersected surface.
Same as general_cube_map_function, but texture coordinates are allowed to be
subtly outside [0,1].
==============================================================================*/
{
#define MAX_UNIT_TEX_COORD   1.000001
#define MIN_UNIT_TEX_COORD  -0.000001
  double t,u,v,sf[3];
	int i;

	ENTER(general_cube_map_function_hack);
	/* if VT spread over several elements, then stretch texture out over these by
		dividing by xi range */
	for (i=0;i<3;i++)
	{
		if (ximax[i]-ximin[i] > 1.0)
		{
			sf[i]=ximax[i] - ximin[i];
		}
		else
		{
			sf[i]=1.0;
		}
	}
	/* check intersect face 5 (z=0) */
	t= -(cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
	u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
	v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
	if ( (t >=0) && (t < 1) && (MIN_UNIT_TEX_COORD <=u) && (u <=MAX_UNIT_TEX_COORD) &&
    (MIN_UNIT_TEX_COORD <=v) && (v <=MAX_UNIT_TEX_COORD) )
	{
		/* valid coordinate */
		*index=4;
		texturemap_coord[0]=(float)1.0 - u;
		texturemap_coord[1]=(float)v;
		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
	}
	else
	{
		/* check intersect face 6 (z=1) */
		t= (1.0 -cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
		u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
		v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
		if ( (t >=0) && (MIN_UNIT_TEX_COORD <=u) && (u <=MAX_UNIT_TEX_COORD) &&
      (MIN_UNIT_TEX_COORD <=v) && (v <=MAX_UNIT_TEX_COORD) )
		{
			/* valid coordinate */
			*index=5;
			texturemap_coord[0]=(float)u;
			texturemap_coord[1]=(float)v;
			/* for present mapped textures are 2d */
			texturemap_coord[2]=0;
		}
		else
		{
			/* check intersect face 1 (x=0) */
			t= -(cop[0]/sf[0])/((vertex_coords[0] - cop[0])/sf[0]);
			u=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
			v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
			if ( (t >=0) && (MIN_UNIT_TEX_COORD <=u) && (u <=MAX_UNIT_TEX_COORD) &&
        (MIN_UNIT_TEX_COORD <=v) && (v <=MAX_UNIT_TEX_COORD) )
			{
				/* valid coordinate */
				*index=0;
				texturemap_coord[0]=(float)u;
				texturemap_coord[1]=(float)v;
				/* for present mapped textures are 2d */
				texturemap_coord[2]=0;
			}
			else
			{
				/* check intersect face 2 (x=1) */
				t= (1.0 -cop[0]/sf[0])/((vertex_coords[0] - cop[0])/sf[0]);
				u=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
				v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
				if ( (t >=0) && (MIN_UNIT_TEX_COORD <=u) && (u <=MAX_UNIT_TEX_COORD) &&
          (MIN_UNIT_TEX_COORD <=v) && (v <=MAX_UNIT_TEX_COORD) )
				{
					/* valid coordinate */
					*index=1;
					texturemap_coord[0]=(float)1.0 - u;
					texturemap_coord[1]=(float)v;
					/* for present mapped textures are 2d */
					texturemap_coord[2]=0;
				}
				else
				{
					/* check intersect face 3 (y=0) */
					t= -(cop[1]/sf[1])/((vertex_coords[1] - cop[1])/sf[1]);
					u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
					v=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
					if ( (t >=0) && (MIN_UNIT_TEX_COORD <=u) && (u <=MAX_UNIT_TEX_COORD) &&
            (MIN_UNIT_TEX_COORD <=v) && (v <=MAX_UNIT_TEX_COORD) )
					{
						/* valid coordinate */
						*index=2;
						texturemap_coord[0]=(float) u;
						texturemap_coord[1]=(float)v;
						/* for present mapped textures are 2d */
						texturemap_coord[2]=0;
					}
					else
					{
						/* check intersect face 4 (y=1) */
						t= (1.0 -cop[1]/sf[1])/((vertex_coords[1] - cop[1])/sf[1]);
						u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
						v=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
						if ( (t >=0) && (MIN_UNIT_TEX_COORD <=u) && (u <=MAX_UNIT_TEX_COORD) &&
              (MIN_UNIT_TEX_COORD <=v) && (v <=MAX_UNIT_TEX_COORD) )
						{
							/* valid coordinate */
							*index=3;
							texturemap_coord[0]=(float)u;
							texturemap_coord[1]=(float)1.0 - v;
							/* for present mapped textures are 2d */
							texturemap_coord[2]=0;
						}
						else
						{
							/* check intersect face 5 (z=0) */
							t= -(cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
							u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
							v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
							if ( (t >=0) && (MIN_UNIT_TEX_COORD <=u) && (u <=MAX_UNIT_TEX_COORD) &&
                (MIN_UNIT_TEX_COORD <=v) && (v <=MAX_UNIT_TEX_COORD) )
							{
								/* valid coordinate */
								*index=4;
								texturemap_coord[0]=(float)1.0 - u;
								texturemap_coord[1]=(float)v;
								/* for present mapped textures are 2d */
								texturemap_coord[2]=0;
							}
							else
							{
								/* check intersect face 6 (z=1) */
								t= (1.0 -cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
								u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
								v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
								if ( (t >=0) && (MIN_UNIT_TEX_COORD <=u) && (u <=MAX_UNIT_TEX_COORD) &&
                  (MIN_UNIT_TEX_COORD <=v) && (v <=MAX_UNIT_TEX_COORD) )
								{

									/* valid coordinate */
									*index=5;
									texturemap_coord[0]=(float)u;
									texturemap_coord[1]=(float)v;
									/* for present mapped textures are 2d */
									texturemap_coord[2]=0;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"general_cube_map_function_hack.  No valid intersection");
								}
							}
						}
					}
				}
			}
		}
	}
	LEAVE;
} /* general_cube_map_function_hack */

void general_cube_map_function(int *index,float *texturemap_coord,
	float *vertex_coords,double cop[3],double ximin[3],double ximax[3])
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Calculates u,v texture map values for a vertex. Third texture coordinate set to
zero (in anticipation of SGI 3d texture mapping potential combination).

Calculates intersection from a centre of projection through a vertex onto the
surface of a cube - returns u,v coords on surface, and index (1-6) of
intersected surface.
==============================================================================*/
{
	double t,u,v,sf[3];
	int i;

	ENTER(general_cube_map_function);
	/* if VT spread over several elements, then stretch texture out over these by
		dividing by xi range */
	for (i=0;i<3;i++)
	{
		if (ximax[i]-ximin[i] > 1.0)
		{
			sf[i]=ximax[i] - ximin[i];
		}
		else
		{
			sf[i]=1.0;
		}
	}
	/* check intersect face 5 (z=0) */
	t= -(cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
	u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
	v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
	if ( (t >=0) && (t < 1) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
	{
		/* valid coordinate */
		*index=4;
		texturemap_coord[0]=(float)1.0 - u;
		texturemap_coord[1]=(float)v;
		/* for present mapped textures are 2d */
		texturemap_coord[2]=0;
	}
	else
	{
		/* check intersect face 6 (z=1) */
		t= (1.0 -cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
		u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
		v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
		if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
		{
			/* valid coordinate */
			*index=5;
			texturemap_coord[0]=(float)u;
			texturemap_coord[1]=(float)v;
			/* for present mapped textures are 2d */
			texturemap_coord[2]=0;
		}
		else
		{
			/* check intersect face 1 (x=0) */
			t= -(cop[0]/sf[0])/((vertex_coords[0] - cop[0])/sf[0]);
			u=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
			v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
			if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
			{
				/* valid coordinate */
				*index=0;
				texturemap_coord[0]=(float)u;
				texturemap_coord[1]=(float)v;
				/* for present mapped textures are 2d */
				texturemap_coord[2]=0;
			}
			else
			{
				/* check intersect face 2 (x=1) */
				t= (1.0 -cop[0]/sf[0])/((vertex_coords[0] - cop[0])/sf[0]);
				u=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
				v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
				if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
				{
					/* valid coordinate */
					*index=1;
					texturemap_coord[0]=(float)1.0 - u;
					texturemap_coord[1]=(float)v;
					/* for present mapped textures are 2d */
					texturemap_coord[2]=0;
				}
				else
				{
					/* check intersect face 3 (y=0) */
					t= -(cop[1]/sf[1])/((vertex_coords[1] - cop[1])/sf[1]);
					u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
					v=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
					if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
					{
						/* valid coordinate */
						*index=2;
						texturemap_coord[0]=(float) u;
						texturemap_coord[1]=(float)v;
						/* for present mapped textures are 2d */
						texturemap_coord[2]=0;
					}
					else
					{
						/* check intersect face 4 (y=1) */
						t= (1.0 -cop[1]/sf[1])/((vertex_coords[1] - cop[1])/sf[1]);
						u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
						v=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
						if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
						{
							/* valid coordinate */
							*index=3;
							texturemap_coord[0]=(float)u;
							texturemap_coord[1]=(float)1.0 - v;
							/* for present mapped textures are 2d */
							texturemap_coord[2]=0;
						}
						else
						{
							/* check intersect face 5 (z=0) */
							t= -(cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
							u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
							v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
							if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
							{
								/* valid coordinate */
								*index=4;
								texturemap_coord[0]=(float)1.0 - u;
								texturemap_coord[1]=(float)v;
								/* for present mapped textures are 2d */
								texturemap_coord[2]=0;
							}
							else
							{
								/* check intersect face 6 (z=1) */
								t= (1.0 -cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
								u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
								v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
								if ( (t >=0) && (0 <=u) && (u <=1.0) && (0 <=v) && (v <=1.0) )
								{

									/* valid coordinate */
									*index=5;
									texturemap_coord[0]=(float)u;
									texturemap_coord[1]=(float)v;
									/* for present mapped textures are 2d */
									texturemap_coord[2]=0;
								}
								else
								{
                  /*???RC Now try the hack version - for numerical rounding
                    problems */
                  general_cube_map_function_hack(index,texturemap_coord,
                    vertex_coords,cop,ximin,ximax);
								}
							}
						}
					}
				}
			}
		}
	}
	LEAVE;
} /* general_cube_map_function */

void general_face_cube_map_function(int *index,float texturemap_coord[3],
	float *vertex_coords,double cop[3],int face_index,double ximin[3],
	double ximax[3])
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Calculates u,v texture map values for a vertex on a face.  This varies from the
general case in that face values are fixed as the projection is made onto them.
==============================================================================*/
{
	double t,u,v,sf[3];
	int i;

	ENTER(general_face_cube_map_function);
	/* if VT spread over several elements, then stretch texture out over these by
		dividing by xi range */
	for (i=0;i<3;i++)
	{
		if (ximax[i]-ximin[i] > 1.0)
		{
			sf[i]=ximax[i] - ximin[i];
		}
		else
		{
			sf[i]=1.0;
		}
	}
	switch (face_index)
	{
		case 1:
		{
			t= -(cop[0]/sf[0])/((vertex_coords[0] - cop[0])/sf[0]);
			u=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
			v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
			*index=0;
			texturemap_coord[0]=(float) u;
			texturemap_coord[1]=(float) v;
			/* for present mapped textures are 2d */
			texturemap_coord[2]=0;
		} break;
		case 2:
		{
			t= (1.0 -cop[0]/sf[0])/((vertex_coords[0] - cop[0])/sf[0]);
			u=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
			v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
			*index=1;
			texturemap_coord[0]=(float)1.0 - u;
			texturemap_coord[1]=(float)v;
			/* for present mapped textures are 2d */
			texturemap_coord[2]=0;
		} break;
		case 3:
		{
			t= -(cop[1]/sf[1])/((vertex_coords[1] - cop[1])/sf[1]);
			u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
			v=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
			*index=2;
			texturemap_coord[0]=(float)u;
			texturemap_coord[1]=(float)v;
			/* for present mapped textures are 2d */
			texturemap_coord[2]=0;
		} break;
		case 4:
		{
			t= (1.0 -cop[1]/sf[1])/((vertex_coords[1] - cop[1])/sf[1]);
			u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
			v=(cop[2] + t*(vertex_coords[2]-cop[2]))/sf[2];
			/* valid coordinate */
			*index=3;
			texturemap_coord[0]=(float)u;
			texturemap_coord[1]=(float)1.0 - v;
			/* for present mapped textures are 2d */
			texturemap_coord[2]=0;
		} break;
		case 5:
		{
			t= -(cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
			u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
			v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
			*index=4;
			texturemap_coord[0]=(float)1.0 - u;
			texturemap_coord[1]=(float)v;
			/* for present mapped textures are 2d */
			texturemap_coord[2]=0;
		} break;
		case 6:
		{
			t= (1.0 -cop[2]/sf[2])/((vertex_coords[2] - cop[2])/sf[2]);
			u=(cop[0] + t*(vertex_coords[0]-cop[0]))/sf[0];
			v=(cop[1] + t*(vertex_coords[1]-cop[1]))/sf[1];
			/* valid coordinate */
			*index=5;
			texturemap_coord[0]=(float)u;
			texturemap_coord[1]=(float)v;
			/* for present mapped textures are 2d */
			texturemap_coord[2]=0;
		} break;
	}
	LEAVE;
} /* general_face_cube_map_function */

void clean_mc_iso_surface(int n_scalar_fields,
	struct MC_iso_surface *mc_iso_surface)
/*******************************************************************************
LAST MODIFIED : 3 February 1998

DESCRIPTION :
==============================================================================*/
{
	int i;
	struct MC_cell *mc_cell;

	ENTER(clean_mc_iso_surface);
	/* clear mc_cell triangles and vertices! */
	if (mc_iso_surface->mc_cells !=NULL)
	{
		for (i=0;i<(mc_iso_surface->dimension[0]+2)*(mc_iso_surface->dimension[1]+2)*
				  (mc_iso_surface->dimension[2]+2);i++)
		{
			if (mc_iso_surface->mc_cells[i] !=NULL)
			{
				destroy_mc_triangle_list(mc_iso_surface->mc_cells[i],
					mc_iso_surface->mc_cells[i]->triangle_list,
					mc_iso_surface->mc_cells[i]->n_triangles,n_scalar_fields,
					mc_iso_surface);
				mc_iso_surface->mc_cells[i]->triangle_list=NULL;
				mc_iso_surface->mc_cells[i]->n_triangles=NULL;
				mc_cell=mc_iso_surface->mc_cells[i];
				DEALLOCATE(mc_cell);
				mc_iso_surface->mc_cells[i]=NULL;
			}
		}
		DEALLOCATE(mc_iso_surface->mc_cells);
	}
	else
	{
		/* SAB If there aren't any cells but there are compiled lists then
			the it may have been read in from a file and each triangle and
			vertex needs to be freed, I have reverse freed them so that the
			debug checking is as efficient as possible */
		for (i = 0 ; i < mc_iso_surface->n_vertices ; i++)
		{
			if(mc_iso_surface->compiled_vertex_list[mc_iso_surface->n_vertices-i-1]
				->triangle_ptrs)
			{
				DEALLOCATE(mc_iso_surface->compiled_vertex_list[mc_iso_surface->n_vertices-i-1]
					->triangle_ptrs);
			}
		}
		for (i = 0 ; i < mc_iso_surface->n_triangles ; i++)
		{
			DEALLOCATE(mc_iso_surface->compiled_triangle_list[mc_iso_surface->n_triangles-i-1]);
		}
		for (i = 0 ; i < mc_iso_surface->n_vertices ; i++)
		{
			DEALLOCATE(mc_iso_surface->compiled_vertex_list[mc_iso_surface->n_vertices-i-1]);
		}
	}
	/*???SAB. Not sure about this but we need to free this somewhere.  Is this
		supposed to only tidy up rather than destroy? */
	DEALLOCATE(mc_iso_surface->compiled_triangle_list);
	DEALLOCATE(mc_iso_surface->compiled_vertex_list);
	DEALLOCATE(mc_iso_surface->detail_map);
	if (mc_iso_surface->deform)
	{
		DEALLOCATE(mc_iso_surface->deform);
	}
	mc_iso_surface->n_vertices=0;
	mc_iso_surface->n_triangles=0;
} /* clean_mc_iso_surface */

int calculate_detail_map(struct VT_volume_texture *texture,
	struct MC_iso_surface *mc_iso_surface)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Fills mcubes detail map 0: leave same 1+: increase trianglular resolution
==============================================================================*/
{
	int dim[3],i,j,k,mcnx,mcny,/*mcnz,*/return_code;

	ENTER(calculate_detail_map);
	if (texture&&mc_iso_surface&&mc_iso_surface->detail_map)
	{
		return_code=1;
		mcnx=mc_iso_surface->dimension[0]+2;
		mcny=mc_iso_surface->dimension[1]+2;
		/*mcnz=mc_iso_surface->dimension[2]+2;*/
#if defined (DEBUG)
		/*???debug */
		printf("mc_iso->detail_map = %p\n", mc_iso_surface->detail_map);
		printf("mcnx = %d mcny = %d mcnz = %d\n", mcnx, mcny, mcnz);
#endif /* defined (DEBUG) */
		for (i=0;i<3;i++)
		{
			dim[i]=texture->dimension[i];
		}
		for (i=0;i<mc_iso_surface->dimension[0];i++)
		{
			for (j=0;j<mc_iso_surface->dimension[1];j++)
			{
				for (k=0;k<mc_iso_surface->dimension[2];k++)
				{
					(mc_iso_surface->detail_map)[(i+1)+(j+1)*mcnx+(k+1)*mcnx*mcny]=
						((texture->texture_cell_list)[i+j*dim[0]+k*dim[0]*dim[1]])->detail;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_detail_map.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_detail_map */
