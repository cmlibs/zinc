/************************************************************************************
   FILE: morphology_functions.c

   LAST MODIFIED: 23 June 2004

   DESCRIPTION: Basic functions for 3D image processing
===================================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"

#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

#include "image_processing/morphology_functions.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))

#undef  A
#define A(X,Y)  *(a + (X)*dim + (Y))
#undef  IN
#define IN(X,Y)  *(in + (X)*dim + (Y))
#define BIGNUM  1.0E15
#define DISTANCE sqrt((x - xctr) * (x - xctr) + (y - yctr) * (y - yctr) + (z - zctr) * (z - zctr))
#define DISTANCE1 sqrt((x - xctr) * (x - xctr) + (y - yctr) * (y - yctr))


extern double sqrt(double x);

extern long int random();

extern	char *setstate();
extern char *initstate();

int Spatial_gray_tone_histogram(FE_value *data_index, int radius, int xctr, int yctr, int zctr,
         int xsize, int ysize, int zsize, int levels, int *h)
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the gray tone histogram at (xctr,yctr,zctr).
=========================================================================================*/
{
        int	lmarg, rmarg;			/* x limits of analysis region	*/
        int	tlay, blay;			/* y limits of analysis region	*/
        int 	top, bottom;			/* z limits of analysis region	*/
	int	ix, iy, iz;			/* integer grid position	*/
	FE_value 	dist;				/* dist. from ctr to grid pt.	*/
        FE_value 	dx, dy, dz;			/* grid position (FE_value)	*/

	FE_value dradius;
	int i, k;
	int 	cur_pt;				/* current location in data_index1	*/
	int neighbor2, neighbor3;
	int neighbor4, neighbor5, neighbor6;
	int neighbor7, neighbor8;

	FE_value *g_map;

	int return_code;

	ENTER(Gray_tone_histogram);

	if (ALLOCATE(g_map, FE_value, xsize * ysize * zsize))
	{
                return_code = 1;

                dradius = (FE_value)radius;
		lmarg = xctr - radius;
                if (lmarg < 0) lmarg = 0;
                rmarg = xctr + radius;
                if (rmarg > (xsize - 1)) rmarg = xsize - 1;

                tlay = yctr - radius;
                if (tlay < 0) tlay = 0;
                blay = yctr + radius;
                if (blay > (ysize - 1)) blay = ysize - 1;

                top = zctr - radius;
                if (top < 0) top = 0;
	        bottom = zctr + radius;
		if (bottom > (zsize - 1)) bottom = zsize - 1;
		for (i = 0; i < xsize * ysize *zsize; i++)
		{
			g_map[i] = 0;
		}
		for (k = 0; k < levels; k++)
		{
		        h[k] = 0;
		}

		for (iz = top; iz < bottom; iz++)
		{
			for (iy = tlay; iy < blay; iy++)
			{
				for (ix = lmarg; ix < rmarg; ix++)
				{
                                        cur_pt = (iz * ysize + iy) * xsize + ix;
					neighbor2 = (iz * ysize + iy) * xsize + ix + 1;
					neighbor3 = (iz * ysize + iy + 1) * xsize + ix;
					neighbor4 = (iz * ysize + iy + 1) * xsize + ix + 1;
					neighbor5 = ((iz + 1) * ysize + iy) * xsize + ix;
					neighbor6 = ((iz + 1) * ysize + iy) * xsize + ix + 1;
					neighbor7 = ((iz + 1) * ysize + iy + 1) * xsize + ix;
					neighbor8 = ((iz + 1) * ysize + iy + 1) * xsize + ix + 1;
					*(g_map + cur_pt) = *(data_index + cur_pt);
					if ((ix + 1) < rmarg)
					{
					        *(g_map + cur_pt) += 2.0 * (*(data_index + neighbor2));
					}
					if ((iy + 1) < blay)
					{
					        *(g_map + cur_pt) += 4.0 * (*(data_index + neighbor3));
					}
					if (((ix + 1) < rmarg) && ((iy + 1) < blay))
					{
					        *(g_map + cur_pt) += 8.0 * (*(data_index + neighbor4));
					}
					if ((iz + 1) < bottom)
					{
					        *(g_map + cur_pt) += 16.0 * (*(data_index + neighbor5));
						if ((ix + 1) < rmarg)
					        {
					                *(g_map + cur_pt) += 32.0 * (*(data_index + neighbor6));
					        }
					        if ((iy + 1) < blay)
					        {
					                *(g_map + cur_pt) += 64.0 * (*(data_index + neighbor7));
					        }
					        if (((ix + 1) < rmarg) && ((iy + 1) < blay))
					        {
					                *(g_map + cur_pt) += 128.0 * (*(data_index + neighbor8));
					        }
					}
				}/* end loop in x */
			}/* end loop in y */
		}/* end loop in z */

                        for (iz = top; iz <= bottom; iz++)
		        {
                	        for (iy = tlay; iy <= blay; iy++)
			        {
                		        for (ix = lmarg; ix <= rmarg; ix++)
				        {
					        dx = (FE_value)ix - (FE_value)xctr;
                		                dy = (FE_value)iy - (FE_value)yctr;
                		                dz = (FE_value)iz - (FE_value)zctr;

	                	                cur_pt = (iz * ysize + iy) * xsize + ix;
						dist = sqrt(dx * dx + dy * dy + dz * dz);
						if(dist <= dradius)
						{
							h[(int)(*(g_map + cur_pt))] += 1;
					        }
				        }
        	                }
                        }

		DEALLOCATE(g_map);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"binary3d_euler_number.  Not enough memory");
			return_code = 0;
	}
	LEAVE;

        return (return_code);
}/* Spatial_gray_tone_histogram */

int Spatial_gray_tone_histogram_cube(FE_value *data_index,
         int xsize, int ysize, int zsize, int levels, int *h)
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the gray tone histogram at (xctr,yctr,zctr).
=========================================================================================*/
{
	int	ix, iy, iz;			/* integer grid position	*/
	int i, k;
	int 	cur_pt;				/* current location in data_index1	*/
	int neighbor2, neighbor3;
	int neighbor4, neighbor5, neighbor6;
	int neighbor7, neighbor8;

	int *g_map;

	int return_code;

	ENTER(Gray_tone_histogram_cube);

	if (ALLOCATE(g_map, int, xsize * ysize * zsize))
	{
                return_code = 1;

		for (i = 0; i < xsize * ysize *zsize; i++)
		{
			g_map[i] = 0;
		}
		for (k = 0; k < levels; k++)
		{
		        h[k] = 0;
		}

		for (iz = 0; iz < zsize; iz++)
		{
			for (iy = 0; iy < ysize; iy++)
			{
				for (ix = 0; ix < xsize; ix++)
				{
                                        cur_pt = (iz * ysize + iy) * xsize + ix;
					neighbor2 = (iz * ysize + iy) * xsize + ix + 1;
					neighbor3 = (iz * ysize + iy + 1) * xsize + ix;
					neighbor4 = (iz * ysize + iy + 1) * xsize + ix + 1;
					neighbor5 = ((iz + 1) * ysize + iy) * xsize + ix;
					neighbor6 = ((iz + 1) * ysize + iy) * xsize + ix + 1;
					neighbor7 = ((iz + 1) * ysize + iy + 1) * xsize + ix;
					neighbor8 = ((iz + 1) * ysize + iy + 1) * xsize + ix + 1;
					*(g_map + cur_pt) = (int)(*(data_index + cur_pt));
					if ((ix + 1) < xsize)
					{
					        *(g_map + cur_pt) += 2 * (int)(*(data_index + neighbor2));
					}
					if ((iy + 1) < ysize)
					{
					        *(g_map + cur_pt) += 4 * (int)(*(data_index + neighbor3));
					}
					if (((ix + 1) < xsize) && ((iy + 1) < ysize))
					{
					        *(g_map + cur_pt) += 8 * (int)(*(data_index + neighbor4));
					}
					if ((iz + 1) < zsize)
					{
					        *(g_map + cur_pt) += 16 * (int)(*(data_index + neighbor5));
						if ((ix + 1) < xsize)
					        {
					                *(g_map + cur_pt) += 32 * (int)(*(data_index + neighbor6));
					        }
					        if ((iy + 1) < ysize)
					        {
					                *(g_map + cur_pt) += 64 * (int)(*(data_index + neighbor7));
					        }
					        if (((ix + 1) < xsize) && ((iy + 1) < ysize))
					        {
					                *(g_map + cur_pt) += 128 * (int)(*(data_index + neighbor8));
					        }
					}
				}/* end loop in x */
			}/* end loop in y */
		}/* end loop in z */

                        for (iz = 0; iz < zsize; iz++)
		        {
                	        for (iy = 0; iy < ysize; iy++)
			        {
                		        for (ix = 0; ix < xsize; ix++)
				        {
					        cur_pt = (iz * ysize + iy) * xsize + ix;
						h[*(g_map + cur_pt)] += 1;
				        }
        	                }
                        }

		DEALLOCATE(g_map);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"binary3d_euler_number.  Not enough memory");
			return_code = 0;
	}
	LEAVE;

        return (return_code);
}/* Spatial_gray_tone_histogram_cube */


int Spatial_Euler_number_old(FE_value pixsize, int *h, FE_value *euler_number)
/*****************************************************************************************
   LAST MODIFIED: 6 May 2004

   DESCRIPTION: Compute the Euler number density.
=========================================================================================*/
{
	FE_value V;
	int k;

	int levels;

	int *theta;
	int Eu_num;

	int iVol = 0;

	int return_code;

	ENTER(Spatial_Euler_number);
	levels = 256;
	if (ALLOCATE(theta, int, levels))
	{
                return_code = 1;
		theta[0] = 0; theta[1] = 1; theta[2] = 0; theta[3] = 0; theta[4] = 0; theta[5] = 0; theta[6] = 0; theta[7] = -1;
			theta[8] = 0; theta[9] = 1; theta[10] = 0; theta[11] = 0; theta[12] = 0; theta[13] = 0; theta[14] = 0; theta[15] = 0;
			theta[16] = 0; theta[17] = 0; theta[18] = 0; theta[19] = -1; theta[20] = 0; theta[21] = -1; theta[22] = 0; theta[23] = -2;
			theta[24] = 0; theta[25] = 0; theta[26] = 0; theta[27] = -1; theta[28] = 0; theta[29] = -1; theta[30] = 0; theta[31] = -1;
			theta[32] = 0; theta[33] = 1; theta[34] = 0; theta[35] = 0; theta[36] = 0; theta[37] = 0; theta[38] = 0; theta[39] = -1;
			theta[40] = 0; theta[41] = 1; theta[42] = 0; theta[43] = 0; theta[44] = 0; theta[45] = 0; theta[46] = 0; theta[47] = 0;
			theta[48] = 0; theta[49] = 0; theta[50] = 0; theta[51] = 0; theta[52] = 0; theta[53] = -1; theta[54] = 0; theta[55] = -1;
			theta[56] = 0; theta[57] = 0; theta[58] = 0; theta[59] = 0; theta[60] = 0; theta[61] = -1; theta[62] = 0; theta[63] = 0;

			theta[64] = 0; theta[65] = 1; theta[66] = 0; theta[67] = 0; theta[68] = 0; theta[69] = 0; theta[70] = 0; theta[71] = -1;
			theta[72] = 0; theta[73] = 1; theta[74] = 0; theta[75] = 0; theta[76] = 0; theta[77] = 0; theta[78] = 0; theta[79] = 0;
			theta[80] = 0; theta[81] = 0; theta[82] = 0; theta[83] = -1; theta[84] = 0; theta[85] = 0; theta[86] = 0; theta[87] = -1;
			theta[88] = 0; theta[89] = 0; theta[90] = 0; theta[91] = -1; theta[92] = 0; theta[93] = 0; theta[94] = 0; theta[95] = 0;
			theta[96] = 0; theta[97] = 1; theta[98] = 0; theta[99] = 0; theta[100] = 0; theta[101] = 0; theta[102] = 0; theta[103] = 0;
			theta[104] = 0; theta[105] = 1; theta[106] = 0; theta[107] = 0; theta[108] = 0; theta[109] = 0; theta[110] = 0; theta[111] = 0;
			theta[112] = 0; theta[113] = 0; theta[114] = 0; theta[115] = 0; theta[116] = 0; theta[117] = 0; theta[118] = 0; theta[119] = 0;
			theta[120] = 0; theta[121] = 0; theta[122] = 0; theta[123] = 0; theta[124] = 0; theta[125] = 0; theta[126] = 0; theta[127] = 1;

			theta[128] = 0; theta[129] = 1; theta[130] = 0; theta[131] = 0; theta[132] = 0; theta[133] = 0; theta[134] = 0; theta[135] = -1;
			theta[136] = 0; theta[137] = 1; theta[138] = 0; theta[139] = 0; theta[140] = 0; theta[141] = 0; theta[142] = 0; theta[143] = 0;
			theta[144] = 0; theta[145] = 0; theta[146] = 0; theta[147] = -1; theta[148] = 0; theta[149] = -1; theta[150] = 0; theta[151] = -2;
			theta[152] = 0; theta[153] = 0; theta[154] = 0; theta[155] = -1; theta[156] = 0; theta[157] = -1; theta[158] = 0; theta[159] = -1;
			theta[160] = 0; theta[161] = 1; theta[162] = 0; theta[163] = 0; theta[164] = 0; theta[165] = 0; theta[166] = 0; theta[167] = -1;
			theta[168] = 0; theta[169] = 1; theta[170] = 0; theta[171] = 0; theta[172] = 0; theta[173] = 0; theta[174] = 0; theta[175] = 0;
			theta[176] = 0; theta[177] = 0; theta[178] = 0; theta[179] = 0; theta[180] = 0; theta[181] = -1; theta[182] = 0; theta[183] = -1;
			theta[184] = 0; theta[185] = 0; theta[186] = 0; theta[187] = 0; theta[188] = 0; theta[189] = -1; theta[190] = 0; theta[191] = 0;

			theta[192] = 0; theta[193] = 1; theta[194] = 0; theta[195] = 0; theta[196] = 0; theta[197] = 0; theta[198] = 0; theta[199] = -1;
			theta[200] = 0; theta[201] = 1; theta[202] = 0; theta[203] = 0; theta[204] = 0; theta[205] = 0; theta[206] = 0; theta[207] = 0;
			theta[208] = 0; theta[209] = 0; theta[210] = 0; theta[211] = -1; theta[212] = 0; theta[213] = 0; theta[214] = 0; theta[215] = -1;
			theta[216] = 0; theta[217] = 0; theta[218] = 0; theta[219] = -1; theta[220] = 0; theta[221] = 0; theta[222] = 0; theta[223] = 0;
			theta[224] = 0; theta[225] = 1; theta[226] = 0; theta[227] = 0; theta[228] = 0; theta[229] = 0; theta[230] = 0; theta[231] = 0;
			theta[232] = 0; theta[233] = 1; theta[234] = 0; theta[235] = 0; theta[236] = 0; theta[237] = 0; theta[238] = 0; theta[239] = 0;
			theta[240] = 0; theta[241] = 0; theta[242] = 0; theta[243] = 0; theta[244] = 0; theta[245] = 0; theta[246] = 0; theta[247] = 0;
			theta[248] = 0; theta[249] = 0; theta[250] = 0; theta[251] = 0; theta[252] = 0; theta[253] = 0; theta[254] = 0; theta[255] = 0;

                Eu_num = 0;

		for (k = 0; k < levels; k++)
		{
		        iVol += h[k];
		        Eu_num += h[k] * theta[k];
		}
		V = (FE_value)iVol * pixsize * pixsize * pixsize;
		*euler_number = (FE_value)(Eu_num)/(24.0 *V);

		DEALLOCATE(theta);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"binary3d_euler_number.  Not enough memory");
			return_code = 0;
	}
	LEAVE;

        return (return_code);
}/* Spatial_Euler_number*/


int Spatial_Euler_number(FE_value pixsize, int *h, FE_value *euler_number)
/*****************************************************************************************
   LAST MODIFIED: 6 May 2004

   DESCRIPTION: Compute the Euler number density.
=========================================================================================*/
{
	FE_value V;
	int k;

	int levels;

	int *theta;
	int Eu_num;

	int iVol = 0;

	int return_code;

	ENTER(Spatial_Euler_number);
	levels = 256;
	if (ALLOCATE(theta, int, levels))
	{
                return_code = 1;
		theta[0] = 0; theta[1] = 1; theta[2] = 0; theta[3] = 0; theta[4] = 0; theta[5] = 0; theta[6] = 0; theta[7] = -1;
			theta[8] = 0; theta[9] = 1; theta[10] = 0; theta[11] = 0; theta[12] = 0; theta[13] = 0; theta[14] = 0; theta[15] = 0;
			theta[16] = 0; theta[17] = 0; theta[18] = 0; theta[19] = -1; theta[20] = 0; theta[21] = -1; theta[22] = 0; theta[23] = -2;
			theta[24] = 0; theta[25] = 0; theta[26] = 0; theta[27] = -1; theta[28] = 0; theta[29] = -1; theta[30] = 0; theta[31] = -1;
			theta[32] = 0; theta[33] = 1; theta[34] = 0; theta[35] = 0; theta[36] = 0; theta[37] = 0; theta[38] = 0; theta[39] = -1;
			theta[40] = 0; theta[41] = 1; theta[42] = 0; theta[43] = 0; theta[44] = 0; theta[45] = 0; theta[46] = 0; theta[47] = 0;
			theta[48] = 0; theta[49] = 0; theta[50] = 0; theta[51] = 0; theta[52] = 0; theta[53] = -1; theta[54] = 0; theta[55] = -1;
			theta[56] = 0; theta[57] = 0; theta[58] = 0; theta[59] = 0; theta[60] = 0; theta[61] = -1; theta[62] = 0; theta[63] = 0;

			theta[64] = 0; theta[65] = 1; theta[66] = 0; theta[67] = 0; theta[68] = 0; theta[69] = 0; theta[70] = 0; theta[71] = -1;
			theta[72] = 0; theta[73] = 1; theta[74] = 0; theta[75] = 0; theta[76] = 0; theta[77] = 0; theta[78] = 0; theta[79] = 0;
			theta[80] = 0; theta[81] = 0; theta[82] = 0; theta[83] = -1; theta[84] = 0; theta[85] = 0; theta[86] = 0; theta[87] = -1;
			theta[88] = 0; theta[89] = 0; theta[90] = 0; theta[91] = -1; theta[92] = 0; theta[93] = 0; theta[94] = 0; theta[95] = 0;
			theta[96] = 0; theta[97] = 1; theta[98] = 0; theta[99] = 0; theta[100] = 0; theta[101] = 0; theta[102] = 0; theta[103] = 0;
			theta[104] = 0; theta[105] = 1; theta[106] = 0; theta[107] = 0; theta[108] = 0; theta[109] = 0; theta[110] = 0; theta[111] = 0;
			theta[112] = 0; theta[113] = 0; theta[114] = 0; theta[115] = 0; theta[116] = 0; theta[117] = 0; theta[118] = 0; theta[119] = 0;
			theta[120] = 0; theta[121] = 0; theta[122] = 0; theta[123] = 0; theta[124] = 0; theta[125] = 0; theta[126] = 0; theta[127] = 1;

			theta[128] = 0; theta[129] = 1; theta[130] = 0; theta[131] = 0; theta[132] = 0; theta[133] = 0; theta[134] = 0; theta[135] = -1;
			theta[136] = 0; theta[137] = 1; theta[138] = 0; theta[139] = 0; theta[140] = 0; theta[141] = 0; theta[142] = 0; theta[143] = 0;
			theta[144] = 0; theta[145] = 0; theta[146] = 0; theta[147] = -1; theta[148] = 0; theta[149] = -1; theta[150] = 0; theta[151] = -2;
			theta[152] = 0; theta[153] = 0; theta[154] = 0; theta[155] = -1; theta[156] = 0; theta[157] = -1; theta[158] = 0; theta[159] = -1;
			theta[160] = 0; theta[161] = 1; theta[162] = 0; theta[163] = 0; theta[164] = 0; theta[165] = 0; theta[166] = 0; theta[167] = -1;
			theta[168] = 0; theta[169] = 1; theta[170] = 0; theta[171] = 0; theta[172] = 0; theta[173] = 0; theta[174] = 0; theta[175] = 0;
			theta[176] = 0; theta[177] = 0; theta[178] = 0; theta[179] = 0; theta[180] = 0; theta[181] = -1; theta[182] = 0; theta[183] = -1;
			theta[184] = 0; theta[185] = 0; theta[186] = 0; theta[187] = 0; theta[188] = 0; theta[189] = -1; theta[190] = 0; theta[191] = 0;

			theta[192] = 0; theta[193] = 1; theta[194] = 0; theta[195] = 0; theta[196] = 0; theta[197] = 0; theta[198] = 0; theta[199] = -1;
			theta[200] = 0; theta[201] = 1; theta[202] = 0; theta[203] = 0; theta[204] = 0; theta[205] = 0; theta[206] = 0; theta[207] = 0;
			theta[208] = 0; theta[209] = 0; theta[210] = 0; theta[211] = -1; theta[212] = 0; theta[213] = 0; theta[214] = 0; theta[215] = -1;
			theta[216] = 0; theta[217] = 0; theta[218] = 0; theta[219] = -1; theta[220] = 0; theta[221] = 0; theta[222] = 0; theta[223] = 0;
			theta[224] = 0; theta[225] = 1; theta[226] = 0; theta[227] = 0; theta[228] = 0; theta[229] = 0; theta[230] = 0; theta[231] = 0;
			theta[232] = 0; theta[233] = 1; theta[234] = 0; theta[235] = 0; theta[236] = 0; theta[237] = 0; theta[238] = 0; theta[239] = 0;
			theta[240] = 0; theta[241] = 0; theta[242] = 0; theta[243] = 0; theta[244] = 0; theta[245] = 0; theta[246] = 0; theta[247] = 0;
			theta[248] = 0; theta[249] = 0; theta[250] = 0; theta[251] = 0; theta[252] = 0; theta[253] = 0; theta[254] = 0; theta[255] = 0;

                Eu_num = 0;

		for (k = 0; k < levels; k++)
		{
		        iVol += h[k];
		        Eu_num += h[k] * theta[k];
		}
		V = (FE_value)iVol * pixsize * pixsize * pixsize;
		*euler_number = (FE_value)(Eu_num)/V;

		DEALLOCATE(theta);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"binary3d_euler_number.  Not enough memory");
			return_code = 0;
	}
	LEAVE;

        return (return_code);
}/* Spatial_Euler_number*/

int Spatial_mean_curvature(FE_value pixsize, int *h, FE_value *mean_curvature)
/*****************************************************************************************
   LAST MODIFIED: 6 May 2004
   DESCRIPTION: Compute the integral of mean curvature.
=========================================================================================*/
{
	int k;

	int levels;

	FE_value *a, M_V = 0.0;
	int ir, ny;
	int iVol = 0;
	int kr[9][4] = {{1,2,4,8},{1,2,16,32},{1,4,16,64},{1,2,64,128},{4,16,8,32},
	                {1,32,4,128},{2,8,16,64},{2,4,32,64},{1,16,8,128}};
	int kt[8][3] = {{1,64,32},{2,16,128},{8,64,32},{4,16,128},
	                {2,4,128},{8,1,64},{2,4,16},{8,1,32}};
	FE_value c[13] = {0.045778, 0.045778, 0.045778, 0.036981, 0.036981, 0.036981,
	                  0.036981, 0.036981, 0.036981, 0.035196, 0.035196, 0.035196, 0.035196};
	FE_value delta01, s;

	int return_code;

	ENTER(Spatial_mean_curvature);
	levels = 256;
	if (ALLOCATE(a, FE_value, 13))
	{
                return_code = 1;
			/* calculate mean curvature */
		delta01 = sqrt(2.0 * pixsize * pixsize);
		s = (3.0 * delta01) / 2.0;
		a[0] = a[1] = a[2] = pixsize * pixsize;
		a[3] = a[4] = a[5] = a[6] = a[7] = a[8] = pixsize * delta01;
		a[9] = a[10] = a[11] = a[12] = 2.0 * sqrt(s * (s - delta01) * (s - delta01) * (s - delta01));
		for (k = 0; k < levels; k++)
		{
			iVol += h[k];
			for (ny = 0; ny < 9; ny++)
			{
				for (ir = 0; ir < 4; ir++)
				{
					M_V += (FE_value)h[k] * c[ny] / (4.0 * a[ny])
						       *((k == (k | kr[ny][ir]))*(0 == (k & kr[ny][(ir + 1) % 4]))
						       *(0 == (k & kr[ny][(ir + 2) % 4])) * (0 == (k & kr[ny][(ir + 3) % 4]))
						       -(k == (k | kr[ny][ir])) * (k == (k | kr[ny][(ir + 1) % 4]))
						       *(k == (k | kr[ny][(ir + 2) % 4])) * (0 == (k & kr[ny][(ir + 3) %4])));
				}
			}
			for (ny = 9; ny < 13; ny++)
			{
				for (ir = 0; ir < 3; ir++)
				{
					M_V += (FE_value)h[k] * c[ny] / (3.0 * a[ny])
						       *((k == (k | kt[ny-9][ir]))*(0 == (k & kt[ny-9][(ir + 1) %3]))
						       *(0 == (k & kt[ny-9][(ir + 2) %3]))
						       -(k == (k | kt[ny-5][ir])) * (k == (k | kt[ny-5][(ir + 1) %3]))
						       *(0 == (k & kt[ny-5][(ir + 2) %3])));
				}
			}
		}
		*mean_curvature = 4.0 * M_PI * M_V / (FE_value)iVol;
		DEALLOCATE(a);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"binary3d_euler_number.  Not enough memory");
			return_code = 0;
	}
	LEAVE;

        return (return_code);
}/* Spatial_mean_curvature*/

int Spatial_gaussian_curvature(FE_value pixsize, int *h, FE_value *total_curvature)
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the integral of total curvature.
=========================================================================================*/
{
	int k;

	int levels;

	int *theta;
	int tcurv;
	int iVol = 0;
	int return_code;

	ENTER(Spatial_Euler_number_and_mean_curvature);
	levels = 256;
	if (ALLOCATE(theta, int, levels))
	{
                return_code = 1;
		theta[0] = 0; theta[1] = 3; theta[2] = 3; theta[3] = 0; theta[4] = 3; theta[5] = 0; theta[6] = 6; theta[7] = -3;
		theta[8] = 3; theta[9] = 6; theta[10] = 0; theta[11] = -3; theta[12] = 0; theta[13] = -3; theta[14] = -3; theta[15] = 0;

		theta[16] = 3; theta[17] = 0; theta[18] = 6; theta[19] = -3; theta[20] = 6; theta[21] = -3; theta[22] = 9; theta[23] = -6;
		theta[24] = 6; theta[25] = 3; theta[26] = 3; theta[27] = -6; theta[28] = 3; theta[29] = -6; theta[30] = 0; theta[31] = -3;

		theta[32] = 3; theta[33] = 6; theta[34] = 0; theta[35] = -3; theta[36] = 6; theta[37] = 3; theta[38] = 3; theta[39] = -6;
		theta[40] = 6; theta[41] = 9; theta[42] = -3; theta[43] = -6; theta[44] = 3; theta[45] = 0; theta[46] = -6; theta[47] = -3;

		theta[48] = 0; theta[49] = -3; theta[50] = -3; theta[51] = 0; theta[52] = 3; theta[53] = -6; theta[54] = 0; theta[55] = -3;
		theta[56] = 3; theta[57] = 0; theta[58] = -6; theta[59] = -3; theta[60] = 0; theta[61] = -8; theta[62] = -8; theta[63] = 0;

		theta[64] = 3; theta[65] = 6; theta[66] = 6; theta[67] = 3; theta[68] = 0; theta[69] = -3; theta[70] = 3; theta[71] = -6;
		theta[72] = 6; theta[73] = 9; theta[74] = 3; theta[75] = 0; theta[76] = -3; theta[77] = -6; theta[78] = -6; theta[79] = -3;

		theta[80] = 0; theta[81] = -3; theta[82] = 3; theta[83] = -6; theta[84] = -3; theta[85] = 0; theta[86] = 0; theta[87] = -3;
		theta[88] = 3; theta[89] = 0; theta[90] = 0; theta[91] = -8; theta[92] = -6; theta[93] = -3; theta[94] = -8; theta[95] = 0;

		theta[96] = 6; theta[97] = 9; theta[98] = 3; theta[99] = 0; theta[100] = 3; theta[101] = 0; theta[102] = 0; theta[103] = -8;
		theta[104] = 9; theta[105] = 12; theta[106] = 0; theta[107] = -3; theta[108] = 0; theta[109] = -3; theta[110] = -8; theta[111] = -6;

		theta[112] = -3; theta[113] = -6; theta[114] = -6; theta[115] = -3; theta[116] = -6; theta[117] = -3; theta[118] = -8; theta[119] = 0;
		theta[120] = 0; theta[121] = -3; theta[122] = -8; theta[123] = -6; theta[124] = -8; theta[125] = -6; theta[126] = -12; theta[127] = 3;

		theta[128] = 3; theta[129] = 6; theta[130] = 6; theta[131] = 3; theta[132] = 6; theta[133] = 3; theta[134] = 9; theta[135] = 0;
		theta[136] = 0; theta[137] = 3; theta[138] = -3; theta[139] = -6; theta[140] = -3; theta[141] = -6; theta[142] = -6; theta[143] = -3;

		theta[144] = 6; theta[145] = 3; theta[146] = 9; theta[147] = 0; theta[148] = 9; theta[149] = 0; theta[150] = 12; theta[151] = -3;
		theta[152] = 3; theta[153] = 0; theta[154] = 0; theta[155] = -8; theta[156] = 0; theta[157] = -8; theta[158] = -3; theta[159] = -6;

		theta[160] = 0; theta[161] = 3; theta[162] = -3; theta[163] = -6; theta[164] = 3; theta[165] = 0; theta[166] = 0; theta[167] = -8;
		theta[168] = -3; theta[169] = 0; theta[170] = 0; theta[171] = -3; theta[172] = -6; theta[173] = -8; theta[174] = -3; theta[175] = 0;

		theta[176] = -3; theta[177] = -6; theta[178] = -6; theta[179] = -3; theta[180] = 0; theta[181] = -8; theta[182] = -3; theta[183] = -6;
		theta[184] = -6; theta[185] = -8; theta[186] = -3; theta[187] = 0; theta[188] = -8; theta[189] = -12; theta[190] = -6; theta[191] = 3;

		theta[192] = 0; theta[193] = 3; theta[194] = 3; theta[195] = 0; theta[196] = -3; theta[197] = -6; theta[198] = 0; theta[199] = -8;
		theta[200] = -3; theta[201] = 0; theta[202] = -6; theta[203] = -8; theta[204] = 0; theta[205] = -3; theta[206] = -3; theta[207] = 0;

		theta[208] = -3; theta[209] = -6; theta[210] = 0; theta[211] = -8; theta[212] = -6; theta[213] = -3; theta[214] = -3; theta[215] = -6;
		theta[216] = -6; theta[217] = -8; theta[218] = -8; theta[219] = -12; theta[220] = -3; theta[221] = 0; theta[222] = -6; theta[223] = 3;

		theta[224] = -3; theta[225] = 0; theta[226] = -6; theta[227] = -8; theta[228] = -6; theta[229] = -8; theta[230] = -8; theta[231] = -12;
		theta[232] = -6; theta[233] = -3; theta[234] = -3; theta[235] = -6; theta[236] = -3; theta[237] = -6; theta[238] = 0; theta[239] = 3;

		theta[240] = 0; theta[241] = -3; theta[242] = -3; theta[243] = 0; theta[244] = -3; theta[245] = 0; theta[246] = -6; theta[247] = 3;
		theta[248] = -3; theta[249] = -6; theta[250] = 0; theta[251] = 3; theta[252] = 0; theta[253] = 3; theta[254] = 3; theta[255] = 0;

                tcurv = 0;

		for (k = 0; k < levels; k++)
		{
		        tcurv += h[k] * theta[k];
			iVol += h[k];
		}

		*total_curvature = (M_PI/6.0)*(FE_value)(tcurv)/((FE_value)iVol *pixsize * pixsize *pixsize);

		DEALLOCATE(theta);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"binary3d_euler_number.  Not enough memory");
			return_code = 0;
	}
	LEAVE;

        return (return_code);
}/* Spatial_gaussian_curvature*/

int Spatial_total_volume(int *h, FE_value *total_volume)
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the tatol volume at the position (xctr,yctr,zctr).
=========================================================================================*/
{
	int k;

	int levels;
	int V = 0;
	int return_code;

	ENTER(Spatial_total_volume);
	levels = 256;
        return_code = 1;

	for (k = 0; k < levels; k++)
	{
		V += h[k];
	}
	*total_volume = (FE_value)V;
	LEAVE;

        return (return_code);
}/* Spatial_total_volume*/

int Spatial_volume_fraction(int *h, FE_value *volume_fraction)
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the volume fraction.
=========================================================================================*/
{
	int k;

	int levels;
	int iVol = 0;
	int iVol1 = 0;
	int return_code;

	ENTER(Spatial_volume_fraction);
	levels = 256;
	return_code = 1;

	for (k = 0; k < levels; k++)
	{
	        iVol += h[k];
		if(k == (k|1))
		{
		       iVol1 += h[k];
		}
	}

	*volume_fraction = (FE_value)(iVol1)/((FE_value)iVol);
	LEAVE;

        return (return_code);
}/* Spatial_volume_fraction*/

int Spatial_surface_density(FE_value pixsize, int *h, FE_value *surface_density)
/*****************************************************************************************
   LAST MODIFIED: 23 April 2004
   DESCRIPTION: Compute the surface density.
=========================================================================================*/
{
	int k;

	int levels;

	FE_value S_V = 0.0;
	int ny;
	int iVol = 0;
	FE_value *r;
	int kl[13][2] = {{1,2},{1,4},{1,16},{1,8},{2,4},{1,32},{2,16},
	                {1,64},{4,16},{1,128},{2,64},{4,32},{8,16}};
	FE_value c[13] = {0.045778, 0.045778, 0.045778, 0.036981, 0.036981, 0.036981,
	                  0.036981, 0.036981, 0.036981, 0.035196, 0.035196, 0.035196, 0.035196};

	int return_code;

	ENTER(Spatial_surface_density);
	levels = 256;
	if (ALLOCATE(r, FE_value, 13))
	{
                return_code = 1;

		r[0] = r[1] = r[2] = pixsize;
		r[3] = r[4] = r[5] = r[6] = r[7] = r[8] = sqrt(2.0*pixsize * pixsize);
		r[9] = r[10] = r[11] = r[12] = sqrt(3.0*pixsize * pixsize);
		for (k = 0; k < levels; k++)
		{
			iVol += h[k];
			for (ny = 0; ny < 13; ny++)
			{
			        S_V += h[k]*c[ny]/r[ny]*((k == (k|kl[ny][0]))*(0 == (k&kl[ny][1]))
				           +(k ==(k|kl[ny][1]))*(0 == (k&kl[ny][0])));
			}
		}
		*surface_density = 6.0 * S_V / (FE_value)iVol;
		DEALLOCATE(r);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"binary3d_euler_number.  Not enough memory");
			return_code = 0;
	}
	LEAVE;

        return (return_code);
}/* Spatial_surface_density*/

int Solve_linear_system (int dim, int order, FE_value *a, FE_value *b, int *pivot)
/************************************************************************
    LAST MODIFIED: 11 February 2004
    DESCRIPTION: solve system of simultaneous eqns.
===========================================================================*/
{
        int i, k, m;
        FE_value temp;
        int return_code;

						/* Forward elimination, trivial case. */
	ENTER(Solve_linear_system);
        if (order == 1)
	{
	       if (A (0, 0) == 0.0)
	       {
	               return_code = 0;
	       }
	       else
	       {
	               b[0] /= A (0, 0);
	               return_code = 1;
	       }
        }
        else
        {
						/* Forward elimination, non-trivial case. */
		return_code = 1;
                for (k = 0; k < order - 1; ++k)
		{
	                m = pivot[k];
	                temp = b[m];
	                b[m] = b[k];
	                b[k] = temp;
	                for (i = k + 1; i < order; ++i)
			{
	                        b[i] += A (i, k) * temp;
	                }
                }
						/* Back substitution */
                for (k = order - 1; k >= 0; --k)
		{
	                if (A (k, k) == 0)
		        {
	                        return_code = 0;
				break;
	                }
		        else
		        {
	                       b[k] /= A (k, k);
	                       temp = -b[k];
	                       for (i = 0; i < k; ++i)
			       {
	                              b[i] += A (i, k) * temp;
	                       }
                        }
                }
	}
	LEAVE;

        return (return_code);

} /* End Solve_linear_system */

int LU_decomp_of_matrix (int dim, int order, FE_value *a, int *pivot, FE_value *condnum)
/**************************************************************************************
    LAST MODIFIED: 11 February 2004

    DESCRIPTION: decompose matrix a into LU matrix
======================================================================================*/
{
						/* Local variable declarations. */
       FE_value *work, ek, temp, anorm, ynorm, znorm;

       int i, j, k, kplus1, m, return_code;

       ENTER(LU_decomp_of_matrix);



        return_code = 1;
						/* Check that "a" matrix order not larger than maximum size */
        if (order <= dim)
	{
                return_code = 1;
						/* Check that "a" matrix order not smaller than one. */
                if (order >= 1)
		{
		        return_code = 1;
					/* Dynamically allocate work vector appropriate for problem. */

                        if (ALLOCATE (work, FE_value, order))
                        {
                                return_code = 1;
                                pivot[order - 1] = 1;
						/* Treat the trivial case. */
                                if (order == 1) {
	                                if (A (0, 0) != 0.0) {
	                                        *condnum = 1.0;
	                                        DEALLOCATE (work);
	                                        return_code = 1;
	                                }
	                                else {
						/* Exact singularity detected for trivial case */
	                                        *condnum = 3.33e33;
	                                        DEALLOCATE (work);
	                                        return_code = 0;
	                                }
                                }
				else
				{
						/* Compute the 1-norm of matrix 'a'.                    */
						/* The 1-norm of matrix 'a' is defined as the largest   */
						/* sum of the absolute values of each of the elements   */
						/* of a single column vector across all column vectors. */
                                        anorm = 0.0;
                                        for (j = 0; j < order; ++j)
					{
	                                        temp = 0.0;
	                                        for (i = 0; i < order; ++i)
						{
	                                                temp += fabs (A (i, j));
	                                        }
	                                        if (temp > anorm)
						{
	                                                anorm = temp;
	                                        }
                                        }
						/* Gaussian elimination with partial pivoting. */
						/* See Linpack function dgefa or sgefa         */
                                        for (k = 0; k < order - 1; ++k)
					{
	                                        kplus1 = k + 1;
						/* Find pivot. */
	                                        m = k;
	                                        for (i = kplus1; i < order; ++i)
						{
	                                                if (fabs (A (i, k)) > fabs (A (m, k)))
							{
		                                                m = i;
	                                                }
	                                        }
	                                        pivot[k] = m;
						/* See Linpack function dgedi or sgedi */
	                                        if (m != k)
						{
	                                                pivot[order - 1] = - pivot[order - 1];
	                                        }
						/* Skip step if pivot is zero. */
	                                        if (A (m, k) != 0.0)
						{
	                                                if (m != k)
							{
		                                               temp = A (m, k);
		                                               A (m, k) = A (k, k);
		                                               A (k, k) = temp;
	                                                }
						/* Compute multipliers. */
	                                                temp = -1.0 / A (k, k);

	                                                for (i = kplus1; i < order; ++i)
							{
		                                               A (i, k) *= temp;
	                                                }
						/* Row elimination with column indexing */
	                                                for (j = kplus1; j < order; ++j)
							{
		                                               temp = A (m, j);
		                                               A (m, j) = A (k, j);
		                                               A (k, j) = temp;
		                                               if (temp != 0.0)
							       {
		                                                      for (i = kplus1; i < order; ++i)
								      {
			                                                      A (i, j) += A (i, k) * temp;
		                                                      }
		                                               }
	                                                }
	                                        }
                                        } /* End k loop. */
						/* Calculate condition number of matrix.                         */
						/* condnum = (1-norm of a)*(an estimate of 1-norm of a-inverse)  */
						/* Estimate obtained by one step of inverse iteration for the    */
						/* small singular vector.  This involves solving two systems     */
						/* of equations, (a-transpose)*y = e and a*z = y where e         */
						/* is a vector of +1 or -1 chosen to cause growth in y.          */
						/* Estimate = (1-norm of z)/(1-norm of y)                        */
						/* First, solve (a-transpose)*y = e                              */
                                        for (k = 0; k < order; ++k)
					{
	                                        temp = 0.0;
	                                        if (k != 0)
						{
	                                                for (i = 0; i < k - 1; ++i)
							{
		                                                temp += A (i, k) * work[i];
	                                                }
	                                        }
	                                        if (temp < 0.0)
						{
	                                                ek = -1.0;
	                                        }
	                                        else
						{
	                                                ek = 1.0;
	                                        }
	                                        if (A (k, k) == 0.0)
						{
						/* Exact singularity detected for non-trivial case */
	                                                *condnum = 3.33e33;
	                                                DEALLOCATE(work);
	                                                return_code = 0;
							break;
	                                        }
	                                        work[k] = -(ek + temp) / A (k, k);
                                        }
					if (return_code)
					{
                                                for (k = order - 2; k >= 0; --k)
						{
	                                                temp = 0.0;
	                                                for (i = k + 1; i < order; ++i)
							{
	                                                         temp += A (i, k) * work[k];
	                                                }
	                                                work[k] = temp;
	                                                m = pivot[k];
	                                                if (m != k)
							{
	                                                         temp = work[m];
	                                                         work[m] = work[k];
	                                                         work[k] = temp;
	                                                }
                                                }

                                                ynorm = 0.0;
                                                for (i = 0; i < order; ++i)
						{
	                                                ynorm += fabs (work[i]);
                                                }
						/* Solve a*z = y. */
                                                return_code = Solve_linear_system (dim, order, a, work, pivot);
                                                if (return_code)
						{
				                        return_code = 1;
                                                        znorm = 0.0;
                                                        for (i = 0; i < order; ++i)
							{
	                                                        znorm += fabs (work[i]);
                                                        }
						/* Estimate condition number. */
                                                        *condnum = anorm * znorm / ynorm;
                                                        if (*condnum < 1.0)
							{
	                                                        *condnum = 1.0;
                                                        }
		                                }
				                else
				                {
				                        display_message(ERROR_MESSAGE, "In Matrix_LU_decomp: error detected in function Linear_system_solve.  "
			                                  "Invalid arguments.");
		                                        return_code = 0;
				                }


				        }
					else
				        {
                                                display_message(ERROR_MESSAGE,"In Matrix_LU_decomp: ""Invalid arguments.");
					        return_code = 0;
				        }

				}
				DEALLOCATE (work);

	                }
	                else
	                {
                               display_message(ERROR_MESSAGE, "In Matrix_LU_decomp: can't allocate work vector.  "
			       "Invalid arguments.");
		               return_code = 0;
	                }
	        }
		else
		{

	                display_message(ERROR_MESSAGE, "In Matrix_LU_decomp: order < 1.  "
			"Invalid arguments.");
		        return_code = 0;

                }
        }
	else
	{

	        display_message(ERROR_MESSAGE, "In Matrix_LU_decomp: order > Max. dim.  "
			"Invalid arguments.");
	        return_code = 0;

        }
        LEAVE;
        return (return_code);

} /* End LU_decomp_of_matrix */

int Inverse_of_matrix (int dim, int order, FE_value *a, FE_value *in)
/*****************************************************************
    LAST MODIFIED: 11 February 2004

    DESCRIPTION: find the inverse of matrix a
=====================================================================*/
{
    int   i, return_code, j;				/* work variables */
    int   *pivot;			/* pivoting from function Matrix_LU_decomp */
    FE_value   *b;				/* B vector */
    FE_value    condnum;			/* matrix condition number */


    ENTER(Inverse_of_matrix);

    return_code = 1;

        if (order <= dim)
	{

                return_code = 1;

						/* allocate work vectors */

                if (ALLOCATE(b, FE_value, order) &&
		        ALLOCATE(pivot, int, order))
		{

		        return_code = 1;
			for (i = 0; i < order; i++)
                                for (j = 0; j < order; j++)
                                        in[i*order +j] = 0.0;
						/* Initialize data values */
                        condnum = 0.0;

						/* decompose matrix */
                        return_code = LU_decomp_of_matrix (dim, order, a, pivot, &condnum);

                        if (return_code)
			{
				return_code = 1;

                                if (condnum <= BIGNUM)
				{
					return_code = 1;
						/* backsolve matrix */
                                        for (i = 0; i < order; ++i)
					{
	                                        for (j = 0; j < order; ++j)
						{
	                                                  if (j == i)
							  {
		                                                   b[j] = 1.0;
	                                                  }
	                                                  else
							  {
		                                                   b[j] = 0.0;
	                                                  }
	                                        }
	                                        return_code = Solve_linear_system (dim, order, a, b, pivot);

	                                        if (return_code)
						{
							  return_code = 1;
	                                                  for (j = 0; j < order; ++j)
							  {
	                                                           IN (j, i) = b[j];
	                                                  }
                                                }
					        else
					        {
							  display_message(ERROR_MESSAGE, "In Inverse, error detected in function Linear_system_solve."
							           " Invalid arguments.");
		                                          return_code = 0;
						          break;
						}
					}

                                }
				else
				{
					display_message(ERROR_MESSAGE, "Inverse: Matrix singular to FE_value precision.  "
			                                 "Invalid arguments.");
		                        return_code = 0;
				}
                        }
			else
			{
				display_message(ERROR_MESSAGE, "In Inverse, error detected in function Matrix_LU_decomp.  "
		                        	"Invalid arguments.");
		                return_code = 0;
			}
			DEALLOCATE (b);
                        DEALLOCATE (pivot);

                }
		else
		{
		        display_message(ERROR_MESSAGE, "In linInverseMatrix, can't allocate memory.  "
			       "Invalid arguments.");
		        return_code = 0;
		}
        }
	else
	{
		display_message(ERROR_MESSAGE, "Inverse: order is greater than dim.  "
			"Invalid arguments.");
		return_code = 0;
	}

	LEAVE;
	return (return_code);

} /* end Inverse_of_matrix */


int Mil_ellipsoid_tensor(FE_value *MIL, FE_value *rott1, FE_value *rott2,
            int number_of_dirs, FE_value *elvector)
/************************************************************************************
    LAST MODIFIED: 11 February 2004

    DESCRIPTION: fit a ellipsoid, return the ellipsoid coeff.
=====================================================================================*/
{

        FE_value	x, y, z;			/* direction vector projections	*/
        FE_value	*a;			/* [number_of_dirs][6],A matrix 			*/
        FE_value	*at;			/* [6][number_of_dirs],A transpose 	(At)		*/
        FE_value	*ata;			/* [6][6],A transpose * A  (AtA)	*/
        FE_value	*atai;			/* [6][6],inverse of AtA  (AtA)^-1	*/
        FE_value	*c;			/* [6][number_of_dirs],(AtA)^-1 * At		*/

        FE_value	*b;			/* [number_of_dirs], vector of 1/(MIL)^2		*/
        FE_value	bmn, bfitmn;			/* mean of measured and fit b	*/
        FE_value	*bfit;			/* [number_of_dirs],b vector based on fit	*/
        FE_value	expdif, expfit, expsqr;		/* stats for measured data	*/
        FE_value	fitsqr, fitdif;			/* stats for fit data		*/
        FE_value        corr,sqres;		/* ellipsoid correlation statistics	*/
        FE_value	stdev,variance;		/* ellipsoid variance statistics	*/
        int	i,j,k, return_code;				/* loop variables 		*/
        int	degree;				/* degree of vectors (6)	*/

	ENTER(Mil_ellipsoid_tensor);

	        degree = 6;
		if (ALLOCATE(a, FE_value, number_of_dirs * degree)&&
                      ALLOCATE(at, FE_value, degree * number_of_dirs)&&
                      ALLOCATE(ata, FE_value, degree * degree)&&
		      ALLOCATE(atai, FE_value, degree * degree)&&
		      ALLOCATE(c, FE_value, degree * number_of_dirs)&&
		      ALLOCATE(b, FE_value, number_of_dirs)&&
		      ALLOCATE(bfit, FE_value, number_of_dirs))
		{
		        return_code = 1;
            /* Initialize the matrices */

	                for (i=0;i<degree;i++)
			{
			        for (j=0;j<degree;j++)
				{
				        ata[i*degree + j]  = 0.0;
					atai[i*degree + j] = 0.0;
				}
			}

		        for (i=0;i<degree;i++)
			{
			        for (k=0;k<number_of_dirs;k++)
				{
		        		c[i * number_of_dirs + k] = 0.0;
			        }
		        }

	        	for (i=0;i<10;i++)
			{
			       elvector[i] = 0.0;
	        	}


                /* Determine A and b matrices of Ax = b */

	        	for (i=0;i<number_of_dirs;i++)
			{
		        	x = cos(rott2[i])*sin(rott1[i]);
		        	y = sin(rott2[i])*sin(rott1[i]);
		        	z = cos(rott1[i]);

	        		a[i * degree + 0] = x * x;
        			a[i * degree + 1] = y * y;
        			a[i * degree + 2] = z * z;
        			a[i * degree + 3] = x * y;
        			a[i * degree + 4] = y * z;
        			a[i * degree + 5] = x * z;

        			b[i] = 1.0 / (*(MIL + i) * *(MIL + i));

	        	}

                /* Determine At of AtAx = Atb */

        		for (i=0;i<number_of_dirs;i++)
			{
	        		at[0 * number_of_dirs + i] = a[i * degree + 0];
	        		at[1 * number_of_dirs + i] = a[i * degree + 1];
	        		at[2 * number_of_dirs + i] = a[i * degree + 2];
	        		at[3 * number_of_dirs + i] = a[i * degree + 3];
	        		at[4 * number_of_dirs + i] = a[i * degree + 4];
	        		at[5 * number_of_dirs + i] = a[i * degree + 5];
	        	}

                /* Multiply At * A */

        		for (i=0;i<degree;i++)
			{
        			for (j=0;j<degree;j++)
				{
	        			for (k=0;k<number_of_dirs;k++)
					{
	        				ata[i * degree + j] += at[i * number_of_dirs + k] * a[k * degree + j];
	        			}
	        		}
	        	}


                /* Find inverse of AtA */
			return_code = Inverse_of_matrix(degree,degree,ata,atai);

	        	if (return_code)
			{
		        	return_code = 1;
                /* Multiply (AtA)^-1 * At */

	                	for (i=0;i<degree;i++)
				{
		                	for (k=0;k<number_of_dirs;k++)
					{
				                for (j=0;j<degree;j++)
						{
				                	c[i * number_of_dirs + k] += atai[i * degree + j]*at[j * number_of_dirs + k];
			                	}
		                	}
	                	}
                /* Multiply (AtA)^-1 * At * b */

	                	for (i=0;i<degree;i++)
				{
		                	for (k=0;k<number_of_dirs;k++)
					{
			                	elvector[i] += c[i * number_of_dirs + k]*b[k];
		                	}
	                	}
                /* Determine the goodness of fit.  Begin by calculating the sum of the	*/
                /* 	square of the residuals. 					*/

         	        	sqres = 0.0;

        	        	for(i=0;i<number_of_dirs;i++)
				{
        	        		bfit[i] = elvector[0]*a[i * degree + 0] + elvector[1]*a[i * degree + 1] +
	        	        	  elvector[2]*a[i * degree + 2] + elvector[3]*a[i * degree + 3] +
	        	        	  elvector[4]*a[i * degree + 4] + elvector[5]*a[i * degree + 5];

		                	sqres += pow((b[i] - bfit[i]), 2.0);
	                	}

                        /*  Find the mean of b[] and bfit[] */

                		bmn = 0.0;
                		bfitmn = 0.0;

                		for(i=0; i<number_of_dirs; i++)
				{
                			bmn += b[i];
                			bfitmn += bfit[i];
	                	}

	                	bmn /= (FE_value)number_of_dirs;
	                	bfitmn /= (FE_value)number_of_dirs;

                        /* Find the variance of the error in the ellipsoid fit */

                		variance = sqres / (FE_value)(number_of_dirs - 6 - 1);

                		stdev	 = sqrt(variance);

                        /*  Calculate the correlation coefficients */

                		expfit=0.0;
	                	expsqr=0.0;
	                	fitsqr=0.0;

	                	for(i=0; i< number_of_dirs; i++)
				{
		                	expdif = b[i] - bmn;
		                	fitdif = bfit[i] - bfitmn;
		                	expfit += expdif * fitdif;
		                	expsqr += expdif * expdif;
		                	fitsqr += fitdif * fitdif;
	                	}

	                	if(( expsqr < 0.0) || (fitsqr < 0.0) )
				{
				        corr = 1.0;  /* no zero div*/
				}
	                	else
				{
				        corr = expfit / sqrt(expsqr*fitsqr);
				}

                        /* Return ellipse coefficients through external variables */

	                	elvector[6] = corr;
	                	elvector[7] = sqres;
	                	elvector[8] = stdev;
	                	elvector[9] = variance;


	        	}
	        	else
	        	{
                        	display_message(ERROR_MESSAGE, "Fiting elliposoid.  "
				"Error computing inverse matrix.");
				return_code = 0;
			}
			DEALLOCATE(a);
	        	DEALLOCATE(ata);
	        	DEALLOCATE(atai);
	        	DEALLOCATE(at);
	        	DEALLOCATE(c);
	        	DEALLOCATE(b);
	        	DEALLOCATE(bfit);
		}
		else
		{
	        	display_message(ERROR_MESSAGE, "Fiting elliposoid.  "
				"Error allocating memory for variables.");
			return_code = 0;
		}


	LEAVE;

        return (return_code);
}/* Mil_ellipsoid_tensor */



int Eigvalue_eigvector_Jacobi (int dim, FE_value *eigvals, FE_value *eigvecs,
           int order, FE_value eps, int *numiters, int maxiters)
/*****************************************************************************
    LAST MODIFIED: 16 February 2004

    DESCRIPTION : Computes the eigenvalues and eigenvectors of
                    a real symmetric matrix via the Jacobi method
============================================================================== */
{

        FE_value vo, f, u, uf, alpha, beta, c, s;

        FE_value *temp1, *temp2, *temp3;
        int i, j, l, m, p, q, return_code;
		/* dynamically allocate temporary vectors */
        ENTER(Eigvalue_eigvector_Jacobi);
        if (ALLOCATE(temp1, FE_value, dim) && ALLOCATE (temp2, FE_value, dim) && ALLOCATE(temp3, FE_value, dim))
	{
                return_code = 1;
						/* make identity matrix */
                for (i = 0; i < order; ++i)
		{
	                for (j = 0; j <= i; ++j)
			{
	                       if (i == j)
			       {
		                       *(eigvecs + i * dim + j) = 1.0;
	                       }
	                       else
			       {
		                       *(eigvecs + i * dim +j) = *(eigvecs + j * dim + i) = 0.0;
	                       }
	                }
                }

                vo = 0.0;
                for (i = 0; i < order; ++i)
		{
	                for (j = 0; j < order; ++j)
			{
	                       if (i != j)
			       {
		                      vo += *(eigvals + i * dim + j) * *(eigvals + i * dim + j);
	                       }
	                }
                }

                u = sqrt (vo) / (FE_value) order;
                uf = eps * u;

                for (uf = eps * u; uf < u; u /= order)
		{
	                for (l = 0; l < order - 1; ++l)
			{
	                        for (m = l + 1; m < order; ++m)
				{
		                        if (fabs (*(eigvals + l * dim + m)) >= u)
					{
		                                p = l;
		                                q = m;
		                                for (i = 0; i < order; ++i)
						{
			                                temp1[i] = *(eigvals + p * dim + i);
			                                temp2[i] = *(eigvals + i * dim + p);
			                                temp3[i] = *(eigvecs + i * dim + p);
		                                }
		                                f = *(eigvals + p * dim + p);
		                                alpha = 0.5 * (*(eigvals + p * dim + p) - *(eigvals + q * dim + q));
		                                beta = sqrt (*(eigvals + p * dim + q) * *(eigvals + p * dim + q) + alpha * alpha);
		                                c = sqrt (0.5 + fabs (alpha) / (2.0 * beta));
		                                s = alpha * (-*(eigvals + p * dim + q)) / (2.0 * beta * fabs (alpha) * c);
		                                for (j = 0; j < order; ++j)
						{
			                                if (j != p)
							{
			                                        if (j != q)
								{
				                                        *(eigvals + p * dim + j) = c * *(eigvals + p * dim + j) - s * *(eigvals + q * dim + j);
				                                        *(eigvals + q * dim + j) = s * temp1[j] + c * *(eigvals + q * dim + j);
				                                        *(eigvals + j * dim + p) = c * *(eigvals + j * dim + p) - s * *(eigvals + q * dim + j);
				                                        *(eigvals + j * dim + q) = s * temp2[j] + c * *(eigvals + j * dim + q);
			                                        }
			                                }
			                                *(eigvecs + j * dim + p) = c * *(eigvecs + j * dim + p) - s * *(eigvecs + j * dim + q);
			                                *(eigvecs + j * dim + q) = s * temp3[j] + c * *(eigvecs + j * dim + q);
		                                }

		                                *(eigvals + p * dim + p) = c * c * *(eigvals + p * dim + p) + s * s *
			                                          *(eigvals + q * dim + q) - 2.0 * c * s * *(eigvals + p * dim + q);
		                                *(eigvals + q * dim + q) = s * s * f + c * c * *(eigvals + q * dim + q) +
			                                          2.0 * c * s * *(eigvals + p * dim + q);
		                                *(eigvals + p * dim + q) = 0.0;
		                                *(eigvals + q * dim + p) = 0.0;

		                                if (++*numiters > maxiters)
						{
                                                        display_message(ERROR_MESSAGE, "eigJacobi.  "
				                             "number of iterations exceeded.");
		                                        return_code = 0;

		                                }
		                        }
	                        }
	                }
                }

                DEALLOCATE (temp1);
                DEALLOCATE (temp2);
                DEALLOCATE (temp3);
        }
        else
        {
                display_message(ERROR_MESSAGE, "eigJacobi.  "
				"Can't allocate memories for temp1, temp2 and temp3.");
		return_code = 0;
        }
	LEAVE;
	return (return_code);

} /* end Eigvalue_eigvector_Jacobi */

int Principal_orientations(FE_value *elvector,
	 FE_value *prin1, FE_value *prin2, FE_value *prin3,
	 FE_value *deg1, FE_value *deg2, FE_value *deg3,
	 FE_value *eigvals, FE_value *eigvecs)
/***********************************************************************
    LAST MODIFIED: 27 April 2004

    DESCRIPTION: Find the eigenvalues and eigenvectors of MIL tensor matrix
==========================================================================*/
{
        FE_value	eps;			/* convergence criterion 		*/
        FE_value	prinmn;			/* mean principal value of MIL		*/

        int	dim,			/* dimension of evalue and evector array*/
	order,			/* order of matrices 			*/
	numiters,		/* number of iterations required	*/
	maxiters,		/* maximum nuber of iterations allowed	*/
	return_code;			/* status returned by eigJacobi		*/

	ENTER(Principal_orientations);
	*prin1 = *prin2 = *prin3 = 0.0;
	*deg1 = *deg2 = *deg3 = 0.0;

	        return_code = 1;
	        dim = 3;
	        order = 3;
	        numiters = 0;
	        maxiters = 50;
	        eps = 1.0e-6;

/* Create the MIL tensor */

	        *(eigvals+0) = elvector[0];
	        *(eigvals+1) = elvector[3]/2.0;
	        *(eigvals+2) = elvector[5]/2.0;
	        *(eigvals+3) = elvector[3]/2.0;
	        *(eigvals+4) = elvector[1];
	        *(eigvals+5) = elvector[4]/2.0;
	        *(eigvals+6) = elvector[5]/2.0;
	        *(eigvals+7) = elvector[4]/2.0;
	        *(eigvals+8) = elvector[2];

/* Calculate the eigenvalues and eigenvectors of the matrix/tensor */

	        return_code = Eigvalue_eigvector_Jacobi(dim,eigvals,eigvecs,order,eps,&numiters,maxiters);

	        if (return_code)
	       {
/* Determine the principal values */

	               *prin1 = 1.000 / sqrt(fabs(*(eigvals+0)));
	               *prin2 = 1.000 / sqrt(fabs(*(eigvals+4)));
	               *prin3 = 1.000 / sqrt(fabs(*(eigvals+8)));


/* Determine the degrees of anisotropy */

	               prinmn = (*prin1 + *prin2 + *prin3)/3.0;

	               *deg1 = (*prin1 - prinmn)/prinmn;
	               *deg2 = (*prin2 - prinmn)/prinmn;
	               *deg3 = (*prin3 - prinmn)/prinmn;
	       }
	       else
	       {
	               display_message(ERROR_MESSAGE,"Could not converge in eigJacobi.");
		       return_code = 0;
	       }

	LEAVE;
	return (return_code);
}/* Principal_orientations */


int Sort_eigenvalue( FE_value *eigvals, FE_value *eigvecs,
	 FE_value *prin1, FE_value *prin2, FE_value *prin3,
	 FE_value *deg1, FE_value *deg2, FE_value *deg3)
/**************************************************************************

      LAST MODIEFIED: 28 April 2004

      DESCRIPTION: Sort the eigenvalues and the corresponding eigenvectore
                    from the largest to the smallest
=========================================================================== */
{

        FE_value ptemp1,ptemp2,ptemp3;

        FE_value dtemp1,dtemp2,dtemp3;

	FE_value *otemp1, *otemp2, *otemp3;

        int return_code;
        ENTER(Sort_eigenvalue);

        if (ALLOCATE(otemp1, FE_value, 3) &&
	       ALLOCATE(otemp2, FE_value, 3) &&
	       ALLOCATE(otemp3, FE_value, 3))
	{
                return_code = 1;
	        ptemp1 = *prin1;
	        ptemp2 = *prin2;
	        ptemp3 = *prin3;

	        otemp1[0] = *(eigvecs+0);
	        otemp1[1] = *(eigvecs+3);
	        otemp1[2] = *(eigvecs+6);

	        otemp2[0] = *(eigvecs+1);
	        otemp2[1] = *(eigvecs+4);
		otemp2[2] = *(eigvecs+7);

		otemp3[0] = *(eigvecs+2);
		otemp3[1] = *(eigvecs+5);
		otemp3[2] = *(eigvecs+8);

	        dtemp1 = *deg1;
	        dtemp2 = *deg2;
	        dtemp3 = *deg3;
               /* Order the data from longest to shortest principal axis */

	       if (ptemp1>ptemp2 && ptemp1>ptemp3)
	       {
		       if (ptemp2>ptemp3)
		       {
			        *prin1 = copysign(ptemp1,*(eigvals+0));
			        *prin2 = copysign(ptemp2,*(eigvals+4));
			        *prin3 = copysign(ptemp3,*(eigvals+8));
			        *(eigvecs + 0) = otemp1[0];
			        *(eigvecs + 3) = otemp1[1];
				*(eigvecs + 6) = otemp1[2];

		                *(eigvecs + 1) = otemp2[0];
		                *(eigvecs + 4) = otemp2[1];
		                *(eigvecs + 7) = otemp2[2];

		                *(eigvecs + 2) = otemp3[0];
		                *(eigvecs + 5) = otemp3[1];
		                *(eigvecs + 8) = otemp3[2];

			        *deg1 = dtemp1;
			        *deg2 = dtemp2;
			        *deg3 = dtemp3;
		        }
		        else
			{
			        *prin1 = copysign(ptemp1,*(eigvals+0));
			        *prin2 = copysign(ptemp3,*(eigvals+8));
			        *prin3 = copysign(ptemp2,*(eigvals+4));

			        *(eigvecs + 0) = otemp1[0];
		                *(eigvecs + 3) = otemp1[1];
		                *(eigvecs + 6) = otemp1[2];

		                *(eigvecs + 1) = otemp3[0];
		                *(eigvecs + 4) = otemp3[1];
		                *(eigvecs + 7) = otemp3[2];
			        *(eigvecs + 2) = otemp2[0];
		                *(eigvecs + 5) = otemp2[1];
				*(eigvecs + 8) = otemp2[2];

			        *deg1 = dtemp1;
			        *deg2 = dtemp3;
			        *deg3 = dtemp2;

		        }
	        }

	        else if (ptemp2>ptemp1 && ptemp2>ptemp3)
		{
		        if (ptemp1>ptemp3)
			{
			        *prin1 = copysign(ptemp2,*(eigvals+4));
			        *prin2 = copysign(ptemp1,*(eigvals+0));
			        *prin3 = copysign(ptemp3,*(eigvals+8));
				*(eigvecs + 0) = otemp2[0];
		                *(eigvecs + 3) = otemp2[1];
		                *(eigvecs + 6) = otemp2[2];

		                *(eigvecs + 1) = otemp1[0];
		                *(eigvecs + 4) = otemp1[1];
		                *(eigvecs + 7) = otemp1[2];

		                *(eigvecs + 2) = otemp3[0];
		                *(eigvecs + 5) = otemp3[1];
		                *(eigvecs + 8) = otemp3[2];

			        *deg1 = dtemp2;
			        *deg2 = dtemp1;
			        *deg3 = dtemp3;

		       }
		       else
		       {
			        *prin1 = copysign(ptemp2,*(eigvals+4));
			        *prin2 = copysign(ptemp3,*(eigvals+8));
			        *prin3 = copysign(ptemp1,*(eigvals+0));

				*(eigvecs + 0) = otemp2[0];
		                *(eigvecs + 3) = otemp2[1];
		                *(eigvecs + 6) = otemp2[2];

		                *(eigvecs + 1) = otemp3[0];
		                *(eigvecs + 4) = otemp3[1];
		                *(eigvecs + 7) = otemp3[2];

		                *(eigvecs + 2) = otemp1[0];
		                *(eigvecs + 5) = otemp1[1];
		                *(eigvecs + 8) = otemp1[2];

			        *deg1 = dtemp2;
			        *deg2 = dtemp3;
			        *deg3 = dtemp1;
		        }
	        }

	        else /*(ptemp3>ptemp1 && ptemp3>ptemp2) */
		{
		        if (ptemp1>ptemp2)
			{
			        *prin1 = copysign(ptemp3,*(eigvals+8));
			        *prin2 = copysign(ptemp1,*(eigvals+0));
			        *prin3 = copysign(ptemp2,*(eigvals+4));

				*(eigvecs + 0) = otemp3[0];
		                *(eigvecs + 3) = otemp3[1];
		                *(eigvecs + 6) = otemp3[2];

		                *(eigvecs + 1) = otemp1[0];
		                *(eigvecs + 4) = otemp1[1];
		                *(eigvecs + 7) = otemp1[2];

		                *(eigvecs + 2) = otemp2[0];
		                *(eigvecs + 5) = otemp2[1];
		                *(eigvecs + 8) = otemp2[2];

			        *deg1 = dtemp3;
			        *deg2 = dtemp1;
			        *deg3 = dtemp2;
		        }
		        else
			{
			        *prin1 = copysign(ptemp3,*(eigvals+8));
			        *prin2 = copysign(ptemp2,*(eigvals+4));
			        *prin3 = copysign(ptemp1,*(eigvals+0));

				*(eigvecs + 0) = otemp3[0];
		                *(eigvecs + 3) = otemp3[1];
		                *(eigvecs + 6) = otemp3[2];

		                *(eigvecs + 1) = otemp2[0];
		                *(eigvecs + 4) = otemp2[1];
		                *(eigvecs + 7) = otemp2[2];

		                *(eigvecs + 2) = otemp1[0];
		                *(eigvecs + 5) = otemp1[1];
		                *(eigvecs + 8) = otemp1[2];

			        *deg1 = dtemp3;
			        *deg2 = dtemp2;
			        *deg3 = dtemp1;
		        }
	        }
		DEALLOCATE(otemp1);
		DEALLOCATE(otemp2);
		DEALLOCATE(otemp3);
	}

	else
	{
                display_message(ERROR_MESSAGE, "sortdata.  "
				"Can't allocate memories for varibles.");
		return_code = 0;
	}
	LEAVE;

        return (return_code);
}/* Sort_eigenvalue */

int Volume_fraction_and_length_of_test_lines(FE_value *data_index, FE_value dradius,
        int xctr, int yctr, int zctr, int iline_sp,
	int xsize, int ysize,
	int lmarg, int rmarg, int tlay, int blay, int top, int bottom,
	FE_value *bv, FE_value *tv, FE_value *tline_ln)
/*****************************************************************************************
   LAST MODIFIED: 23 February 2004
   DESCRIPTION: Compute the bone volume,tatol volume and the length of all test lines at
                the position (xctr,yctr,zctr).
=========================================================================================*/
{

	int	ix, iy, iz;			/* integer grid position	*/
	FE_value 	dist;				/* dist. from ctr to grid pt.	*/
        FE_value 	dx, dy, dz;			/* grid position (FE_value)	*/

        int 	cur_pt;				/* current location in data_index1	*/
	int return_code;


	ENTER(Volume_fraction_and_length_of_test_lines);

	return_code = 1;

                /*  Calculate the solid volume (#voxels) of the test region and	*/

        *bv = *tv = 0.0;


        for(iz = top; iz <= bottom; iz++)
	{
                for (iy = tlay; iy <= blay; iy++)
		{
                	for (ix = lmarg; ix <= rmarg; ix++)
			{
                		dx = (FE_value)ix - (FE_value)xctr;
                		dy = (FE_value)iy - (FE_value)yctr;
                		dz = (FE_value)iz - (FE_value)zctr;

	                	cur_pt = (iz * ysize + iy) * xsize + ix;

        		        	/* Determine # of BONE pixels */

        		        dist = sqrt(dx * dx + dy * dy + dz * dz);
        		        if(dist <= dradius)
				{
        		        	(*tv)++;
        		        	if (*(data_index + cur_pt) == 1.0)
				        {
						(*bv)++;
					}
        		        	else
					{
						*(data_index + cur_pt) = 0.0;
					}
        		        }
        	        }
                }
        }
	 *tline_ln = 0.0;
        for(ix = lmarg; ix <= rmarg; ix += iline_sp)
	{
                dx = (FE_value)ix - (FE_value)xctr;
                for(iy = tlay; iy <= blay; iy += iline_sp)
		{
                	dy = (FE_value)iy - (FE_value)yctr;
                	dist = sqrt(dx * dx + dy * dy);
        	        if(dist <= dradius)
			{
        	        	dz = sqrt(dradius * dradius - dist*dist);
        	        	*tline_ln += 2.0 * dz;
        	        }
                }
	}
	LEAVE;

        return (return_code);
}/* Volume_fraction_and_length_of_test_lines */

int Length_of_test_lines(FE_value dradius,
        int xctr, int yctr, int iline_sp,
	int lmarg, int rmarg, int tlay, int blay,
	FE_value *tline_ln)
/*****************************************************************************************
   LAST MODIFIED: 23 February 2004
   DESCRIPTION: Compute the length of all test lines at
                the position (xctr,yctr,zctr).
=========================================================================================*/
{

	int	ix, iy;			/* integer grid position	*/
	FE_value 	dist;				/* dist. from ctr to grid pt.	*/
        FE_value 	dx, dy, dz;			/* grid position (FE_value)	*/
	int return_code;


	ENTER(Length_of_test_lines);

	return_code = 1;

	 *tline_ln = 0.0;
        for(ix = lmarg; ix <= rmarg; ix += iline_sp)
	{
                dx = (FE_value)ix - (FE_value)xctr;
                for(iy = tlay; iy <= blay; iy += iline_sp)
		{
                	dy = (FE_value)iy - (FE_value)yctr;
                	dist = sqrt(dx * dx + dy * dy);
        	        if(dist <= dradius)
			{
        	        	dz = sqrt(dradius * dradius - dist*dist);
        	        	*tline_ln += 2.0 * dz;
        	        }
                }
	}
	LEAVE;

        return (return_code);
}/* Length_of_test_lines */

int MIL_vector(FE_value *data_index, int number_of_dirs, FE_value dradius,
       FE_value *rot1, FE_value *rot2, FE_value tline_ln, int iline_sp, FE_value bvtv,
       int xsize, int ysize, int xctr, int yctr, int zctr,
       int lmarg, int rmarg, int tlay, int blay, int top, int bottom,
       FE_value *mil, int *tintrsctn, int *num_tlines)
/*****************************************************************************************
   LAST MODIFIED: 29 March 2004
   DESCRIPTION: Compute the MIL vector, total number of intersections and total number of lines.
=========================================================================================*/
{
        FE_value	theta, phi;			/* orientation angles		*/
        FE_value	sin_theta, cos_theta;		/* trig calculations		*/
        FE_value	sin_phi, cos_phi;		/* more trig calculations	*/
        FE_value	ctcp, ctsp;			/* and more trig calculations	*/
        FE_value 	r[3], norm_r[3];		/* test line direction vectors	*/
	FE_value 	x, y, z;			/* position on test line	*/
        FE_value 	x_off, y_off, z_off;		/* offset to get to start pos.	*/
        FE_value 	dist;				/* dist. from ctr to grid pt.	*/
        FE_value 	dx, dy, dz;			/* grid position (FE_value)	*/
        int	ix, iy;			/* integer grid position	*/

        int 	intrsctn;			/* number of intersections	*/
        int 	cur_pt;				/* current location in data_index1	*/
        FE_value  preval, curval;			/* previous voxel value		*/

        int	irot;				/* rotation loop counter	*/
	FE_value pl;
	int i;
	/*   Scan the image for each of the randomly selected rotations  */

	int return_code = 1;

	ENTER(MIL_vector);

        for( irot = 0; irot < number_of_dirs; irot++)
	{

                theta = rot1[irot];
                phi = rot2[irot];

                sin_phi = sin(phi);
                cos_phi = cos(phi);
                sin_theta = sin(theta);
                cos_theta = cos(theta);
                ctcp = cos_theta*cos_phi;
                ctsp = cos_theta*sin_phi;

                       /*   Define direction vector for this rotation 	*/

                r[0] = cos_phi*sin_theta;
                r[1] = sin_phi*sin_theta;
                r[2] = cos_theta;

                      /*   Normalize direction vector to +1 for the maximum component */

                if ((fabs(r[0]) >= fabs(r[1]))&&(fabs(r[0]) >= fabs(r[2])))
		{

        		norm_r[1] = r[1] / r[0];
                        norm_r[2] = r[2] / r[0];
			norm_r[0] = 1.0;
                }
                else if ((fabs(r[1]) >= fabs(r[0]))&&(fabs(r[1]) >= fabs(r[2])))
		{
        		norm_r[0] = r[0] / r[1];
                        norm_r[2] = r[2] / r[1];
			norm_r[1] = 1.0;
                }
        	else
		{
                	norm_r[0] = r[0] / r[2];
                        norm_r[1] = r[1] / r[2];
			norm_r[2] = 1.0;
                }

                                /*   Make sure signs are correct */

                for (i= 0; i<3; i++)
		{
                        if (r[i] > 0)
			{
				norm_r[i] = fabs(norm_r[i]);
			}
			else
			{
			        norm_r[i] = (-fabs(norm_r[i]));
			}
                }
        	intrsctn = 0;
		(*tintrsctn) = 0;
		(*num_tlines) = 0;

                        /*  Loop over the test grid which is equally spaced in x and y */

                for(ix = lmarg; ix < rmarg; ix += iline_sp)
		{
        		for(iy = tlay; iy < blay; iy += iline_sp)
			{

                        	dx = (FE_value)ix - (FE_value)xctr;
                        	dy = (FE_value)iy - (FE_value)yctr;
                        	dist = sqrt(dx * dx + dy * dy);

                        	if(dist < dradius)
				{
                        		dz = sqrt(dradius * dradius - dist * dist);
                        		x_off = dz * r[0];
                        		y_off = dz * r[1];
                        		z_off = dz * r[2];

                	        	x = dx*ctcp - dy*sin_phi + (FE_value)xctr;
                	        	y = dx*ctsp + dy*cos_phi + (FE_value)yctr;
                	        	z = (-dx)*sin_theta + (FE_value)zctr;

                		        	/* Starting coordinate for this rotation */
                		        x -= x_off;
                		        y -= y_off;
                		        z -= z_off;

                		        	/* First determine whether the line starts in 	*/
        	        	        	/* bone or background		 		*/

                		        cur_pt = ((int)(z)*ysize + (int)(y))*xsize +
					(int)(x);
					if ((int)(z) >= top && (int)(z) <= bottom)
                			{
					        preval = *(data_index + cur_pt);
					}
					else
					{
					        preval = 0.0;
					}

                		        	/* Increment current position along the test line */
                	        	x += norm_r[0];
                	        	y += norm_r[1];
                	        	z += norm_r[2];

                		        while (DISTANCE <= dradius)
					{
                				cur_pt = ((int)(z) * ysize +
        	                			(int)(y)) * xsize + (int)(x);

        		        		/* Count an intersection if the current	*/
		        		        /* voxel value differs from the previous*/
		        		        /* value.				*/
						if ((int)(z) >= top && (int)(z) <= bottom)
                			        {
						        curval = *(data_index + cur_pt);
						}
					        else
					        {
						        curval = 0.0;
						}
	                			if(curval != preval)
						{
	                			        intrsctn += 1;
	                			        preval = curval;
	                			}
	                		        	x += norm_r[0];
	                		        	y += norm_r[1];
	                		        	z += norm_r[2];

	                		} /* end while on line loop */

                			(*num_tlines)++;

                		} /* end loop over z axis */

        		} /* end loop over y axis */

                } /* end loop over x axis */

                       /* Calculate the number of intersections per test line length and the 	*/
                       /*	mean intercept length (MIL) for this rotation.			*/

                pl = (FE_value)intrsctn / tline_ln;
        	mil[irot] = (2.0 * bvtv) / pl;
        	(*tintrsctn) += intrsctn;

        }   /* end loop over number_of_dirs rotations */
	LEAVE;
	return (return_code);

}/* MIL_vector */

