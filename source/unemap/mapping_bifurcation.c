/*******************************************************************************
FILE : mapping.c

LAST MODIFIED : 1 December 1995

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include "myio.h"
#include <string.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include "mapping.h"
#include "debug.h"
#include "mymemory.h"
#include "message.h"
#include "geometry.h"
#include "rig.h"
#include "user_interface.h"
#include "interpolate.h"
#include "postscript.h"

/*
Prototype "cardiac database"
----------------------------
*/
	/*???DB.  Should be read from a file */
#define NUMBER_OF_FIBRE_ROWS 3
#define NUMBER_OF_FIBRE_COLUMNS 4
#define LANDMARK_FOCUS 35.25
#define NUMBER_OF_LANDMARK_POINTS 472
#define FIBRE_TOLERANCE 1.e-12
#define FIBRE_MAXIMUM_ITERATIONS 10

struct Fibre_node
	/*???DB temporary module type */
{
	float mu,theta,fibre_angle;
}; /* struct Fibre_node */

static struct Fibre_node 
	global_fibre_nodes[(NUMBER_OF_FIBRE_ROWS+1)*NUMBER_OF_FIBRE_COLUMNS]=
	{
/*
		{2.0420,6.0214,-1.3273},
		{2.0246,1.3090,-0.6512},
		{1.9373,2.8798,-1.3145},
		{2.0246,4.4506,-0.0443},
		{1.5708,1.0472,-0.8895},
		{1.5708,1.3963,-0.5127},
		{1.5708,3.2463,-1.8455},
		{1.5708,5.0964, 0.3543},
		{0.9599,0.5934,-0.9177},
		{0.9599,1.2741,-1.1602},
		{0.9599,3.7350,-0.7749},
		{0.9599,6.1959,-1.0111},
		{0.0000,0.5934,-1.3813},
		{0.0000,1.2741,-1.3813},
		{0.0000,3.7350,-1.3813},
		{0.0000,6.1959,-1.3813},
*/
		{2.0420,0.5934, 0.7854},
		{2.0420,1.2741, 0.7854},
		{2.0420,3.7350, 0.7854},
		{2.0420,6.1959, 0.7854},
		{1.5708,0.5934, 0.7854},
		{1.5708,1.2741, 0.7854},
		{1.5708,3.7350, 0.7854},
		{1.5708,6.1959, 0.7854},
		{0.9599,0.5934, 0.7854},
		{0.9599,1.2741, 0.7854},
		{0.9599,3.7350, 0.7854},
		{0.9599,6.1959, 0.7854},
		{0.0000,0.5934, 0.7854},
		{0.0000,1.2741, 0.7854},
		{0.0000,3.7350, 0.7854},
		{0.0000,6.1959, 0.7854},
	};

#define NUMBER_OF_BIFURCATION_POINTS 30
static float bifurcation_points[3*NUMBER_OF_BIFURCATION_POINTS]=
	{
		/* bifurcation 1, starts as lad_1, splits into lad_2 and lad_5 */
		-0.331E+02,-0.745E+01,-0.165E+02,

		/* bifurcation 2, starts as lad_2, splits into lad_3 and lad_39 */
		-0.293E+02,-0.745E+01,-0.237E+02,

		/* bifurcation 3, starts as lad_5, splits into lad_6 and lad_7 */
		-0.259E+02,-0.261E+02,-0.125E+02,

		/* bifurcation 4, starts as pda_1, splits into pda_2 and pda_5 */
		-0.363E+02, 0.152E+02,-0.174E+02,

		/* bifurcation 5, starts as lad_6, splits into lad_8 and lad_34 */
		-0.207E+02,-0.327E+02,-0.387E+01,

		/* bifurcation 6, starts as lad_7, splits into lad_9 and lad_19 */
		-0.580E+01,-0.329E+02,-0.181E+02,

		/* bifurcation 7, starts as lad_8, splits into lad_10 and lad_22 */
		-0.169E+02,-0.311E+02, 0.101E+02,

		/* bifurcation 8, starts as lad_9, splits into lad_11 and lad_15 */
		 0.460E+01,-0.299E+02,-0.237E+02,

		/* bifurcation 9, starts as lad_3, splits into lad_12 and lad_4 */
		-0.279E+02,-0.773E+01,-0.265E+02,

		/* bifurcation 10, starts as pda_2, splits into pda_3 and pda_6 */
		-0.359E+02, 0.208E+02,-0.126E+02,

		/* bifurcation 11, starts as lad_10, splits into lad_13 and lad_35 */
		-0.168E+02,-0.179E+02, 0.244E+02,

		/* bifurcation 12, starts as lad_12, splits into lad_14 and lad_40 */
		-0.182E+02,-0.736E+01,-0.363E+02,

		/* bifurcation 13, starts as pda_3, splits into pda_4 and pda_7 */
		-0.341E+02, 0.395E+02, 0.347E+01,

		/* bifurcation 14, starts as lad_15, splits into lad_16 and lad_17 */
		 0.145E+02,-0.289E+02,-0.213E+02,

		/* bifurcation 15, starts as lad_19, splits into lad_20 and lad_21 */
		 0.172E+02,-0.326E+02,-0.432E+01,

		/* bifurcation 16, starts as lad_14, splits into lad_18 and lad_41 */
		-0.880E+01,-0.322E+01,-0.396E+02,

		/* bifurcation 17, starts as lad_22, splits into lad_23 and lad_30 */
		-0.110E+01,-0.242E+02, 0.235E+02,

		/* bifurcation 18, starts as pda_7, splits into pda_8 and pda_13 */
		-0.224E+02, 0.446E+02, 0.932E+01,

		/* bifurcation 19, starts as lad_23, splits into lad_24 and lad_37 */
		 0.231E+02,-0.168E+02, 0.203E+02,

		/* bifurcation 20, starts as lad_18, splits into lad_25 and lad_42 */
		 0.690E+01, 0.108E+02,-0.380E+02,

		/* bifurcation 21, starts as lad_25, splits into lad_26 and lad_43 */
		 0.110E+02, 0.131E+02,-0.362E+02,

		/* bifurcation 22, starts as lad_26, splits into lad_27 and lad_44 */
		 0.290E+02, 0.140E+02,-0.275E+02,

		/* bifurcation 23, starts as pda_8, splits into pda_9 and pda_12 */
		-0.159E+02, 0.447E+02, 0.130E+02,

		/* bifurcation 24, starts as lad_27, splits into lad_28 and lad_38 */
		 0.404E+02, 0.796E+01,-0.206E+02,

		/* bifurcation 25, starts as pda_9, splits into pda_10 and pda_11 */
		 0.150E+01, 0.427E+02, 0.167E+02,

		/* bifurcation 26, starts as lad_30, splits into lad_31 and lad_47 */
		 0.277E+02,-0.246E+02, 0.825E+01,

		/* bifurcation 27, starts as lad_31, splits into lad_32 and lad_33 */
		 0.469E+02,-0.136E+02, 0.673E-01,

		/* bifurcation 28, starts as lad_44, splits into lad_45 and lad_46 */
		 0.361E+02, 0.159E+02,-0.198E+02,

		/* bifurcation 29, starts as pda_13, splits into pda_14 and pda_15 */
		-0.147E+02, 0.472E+02, 0.513E+01,

		/* bifurcation 30, starts as lad_28, splits into lad_29 and lad_36 */
		 0.452E+02, 0.367E+01,-0.181E+02,
	};

static float landmark_points[3*NUMBER_OF_LANDMARK_POINTS]=
	{
		/* lad_1, no starting bifurcation, bifurcates at 1 */
		-0.340E+02,-0.262E+01,-0.146E+02,
		-0.339E+02,-0.453E+01,-0.160E+02,

		/* bifurcation 1, starts as lad_1, splits into lad_2 and lad_5 */
		-0.331E+02,-0.745E+01,-0.165E+02,

		/* lad_2, starts at 1, bifurcates at 2 */
		-0.317E+02,-0.720E+01,-0.191E+02,
		-0.297E+02,-0.753E+01,-0.228E+02,

		/* bifurcation 2, starts as lad_2, splits into lad_3 and lad_39 */
		-0.293E+02,-0.745E+01,-0.237E+02,

		/* lad_3, starts at 2, bifurcates at 9 */
			/*???DB.  NB no points */

		/* bifurcation 9, starts as lad_3, splits into lad_12 and lad_4 */
		-0.279E+02,-0.773E+01,-0.265E+02,

		/* lad_12, starts at 9, bifurcates at 12 */
		-0.256E+02,-0.801E+01,-0.297E+02,
		-0.230E+02,-0.801E+01,-0.324E+02,
		-0.209E+02,-0.787E+01,-0.339E+02,

		/* bifurcation 12, starts as lad_12, splits into lad_14 and lad_40 */
		-0.182E+02,-0.736E+01,-0.363E+02,

		/* lad_14, starts at 12, bifurcates at 16 */
		-0.147E+02,-0.603E+01,-0.378E+02,
		-0.119E+02,-0.484E+01,-0.391E+02,
		-0.960E+01,-0.353E+01,-0.399E+02,

		/* bifurcation 16, starts as lad_14, splits into lad_18 and lad_41 */
		-0.880E+01,-0.322E+01,-0.396E+02,

		/* lad_18, starts at 16, bifurcates at 20 */
		-0.650E+01,-0.130E+01,-0.404E+02,
		-0.370E+01, 0.399E+00,-0.409E+02,
		-0.160E+01, 0.295E+01,-0.405E+02,
		 0.600E+00, 0.519E+01,-0.401E+02,
		 0.260E+01, 0.740E+01,-0.396E+02,
		 0.470E+01, 0.927E+01,-0.389E+02,

		/* bifurcation 20, starts as lad_18, splits into lad_25 and lad_42 */
		 0.690E+01, 0.108E+02,-0.380E+02,

		/* lad_25, starts at 20, bifurcates at 21 */
		 0.880E+01, 0.119E+02,-0.372E+02,

		/* bifurcation 21, starts as lad_25, splits into lad_26 and lad_43 */
		 0.110E+02, 0.131E+02,-0.362E+02,

		/* lad_26, starts at 21, bifurcates at 22 */
		 0.135E+02, 0.136E+02,-0.353E+02,
		 0.156E+02, 0.139E+02,-0.347E+02,
		 0.176E+02, 0.143E+02,-0.336E+02,
		 0.198E+02, 0.146E+02,-0.324E+02,
		 0.220E+02, 0.148E+02,-0.312E+02,
		 0.244E+02, 0.148E+02,-0.301E+02,
		 0.267E+02, 0.144E+02,-0.287E+02,

		/* bifurcation 22, starts as lad_26, splits into lad_27 and lad_44 */
		 0.290E+02, 0.140E+02,-0.275E+02,

		/* lad_27, starts at 22, bifurcates at 24 */
		 0.314E+02, 0.128E+02,-0.260E+02,
		 0.339E+02, 0.115E+02,-0.250E+02,
		 0.361E+02, 0.102E+02,-0.239E+02,
		 0.384E+02, 0.907E+01,-0.221E+02,

		/* bifurcation 24, starts as lad_27, splits into lad_28 and lad_38 */
		 0.404E+02, 0.796E+01,-0.206E+02,

		/* lad_28, starts at 24, bifurcates at 30 */
		 0.428E+02, 0.599E+01,-0.190E+02,

		/* bifurcation 30, starts as lad_28, splits into lad_29 and lad_36 */
		 0.452E+02, 0.367E+01,-0.181E+02,

		/* lad_29, starts at 30, does not bifurcate */
		 0.473E+02, 0.217E+01,-0.149E+02,
		 0.488E+02, 0.893E+00,-0.131E+02,
		 0.501E+02,-0.591E+00,-0.116E+02,

		/* lad_36, starts at 30, does not bifurcate */
		 0.460E+02, 0.184E+01,-0.167E+02,
		 0.464E+02,-0.472E-01,-0.168E+02,
		 0.463E+02,-0.158E+01,-0.167E+02,
		 0.459E+02,-0.310E+01,-0.165E+02,
		 0.450E+02,-0.533E+01,-0.178E+02,

		/* lad_38, starts at 24, does not bifurcate */
		 0.421E+02, 0.766E+01,-0.190E+02,
		 0.440E+02, 0.740E+01,-0.173E+02,
		 0.453E+02, 0.763E+01,-0.157E+02,
		 0.466E+02, 0.758E+01,-0.138E+02,
		 0.481E+02, 0.714E+01,-0.114E+02,
		 0.489E+02, 0.743E+01,-0.974E+01,
		 0.498E+02, 0.785E+01,-0.808E+01,
		 0.505E+02, 0.774E+01,-0.644E+01,

		/* lad_39, starts at 2, does not bifurcate */
		-0.273E+02,-0.826E+01,-0.256E+02,
		-0.253E+02,-0.963E+01,-0.292E+02,
		-0.234E+02,-0.100E+02,-0.304E+02,
		-0.209E+02,-0.110E+02,-0.324E+02,
		-0.184E+02,-0.118E+02,-0.335E+02,
		-0.154E+02,-0.130E+02,-0.345E+02,
		-0.131E+02,-0.143E+02,-0.350E+02,
		-0.108E+02,-0.151E+02,-0.353E+02,
		-0.820E+01,-0.160E+02,-0.355E+02,
		-0.560E+01,-0.162E+02,-0.359E+02,
		-0.310E+01,-0.164E+02,-0.363E+02,
		-0.200E+00,-0.161E+02,-0.364E+02,
		 0.210E+01,-0.161E+02,-0.363E+02,
		 0.510E+01,-0.161E+02,-0.356E+02,

		/* lad_40, starts at 12, does not bifurcate */
		-0.158E+02,-0.751E+01,-0.370E+02,
		-0.128E+02,-0.788E+01,-0.375E+02,
		-0.960E+01,-0.781E+01,-0.385E+02,
		-0.740E+01,-0.787E+01,-0.388E+02,
		-0.460E+01,-0.819E+01,-0.390E+02,
		-0.140E+01,-0.850E+01,-0.391E+02,
		 0.190E+01,-0.905E+01,-0.390E+02,
		 0.510E+01,-0.100E+02,-0.383E+02,

		/* lad_4, starts at 9, does not bifurcate */
		-0.271E+02,-0.631E+01,-0.281E+02,
		-0.281E+02,-0.600E+01,-0.307E+02,
		-0.286E+02,-0.513E+01,-0.337E+02,
		-0.281E+02,-0.316E+01,-0.358E+02,
		-0.267E+02,-0.174E+01,-0.378E+02,
		-0.244E+02,-0.715E+00,-0.393E+02,
		-0.214E+02, 0.112E+00,-0.405E+02,
		-0.192E+02, 0.242E+01,-0.411E+02,
		-0.179E+02, 0.429E+01,-0.425E+02,
		-0.163E+02, 0.644E+01,-0.427E+02,

		/* lad_41, starts at 16, does not bifurcate */
		-0.600E+01,-0.329E+01,-0.405E+02,
		-0.310E+01,-0.273E+01,-0.405E+02,
		-0.100E+00,-0.216E+01,-0.405E+02,
		 0.190E+01,-0.129E+01,-0.401E+02,
		 0.500E+01,-0.184E+01,-0.399E+02,
		 0.790E+01,-0.265E+01,-0.393E+02,
		 0.109E+02,-0.315E+01,-0.388E+02,
		 0.137E+02,-0.307E+01,-0.378E+02,
		 0.165E+02,-0.297E+01,-0.365E+02,
		 0.191E+02,-0.340E+01,-0.356E+02,
		 0.217E+02,-0.226E+01,-0.337E+02,
		 0.246E+02,-0.263E+01,-0.324E+02,
		 0.267E+02,-0.323E+01,-0.315E+02,
		 0.290E+02,-0.386E+01,-0.296E+02,
		 0.312E+02,-0.458E+01,-0.287E+02,
		 0.329E+02,-0.575E+01,-0.274E+02,

		/* lad_42, starts at 20, does not bifurcate */
		 0.860E+01, 0.134E+02,-0.364E+02,
		 0.104E+02, 0.163E+02,-0.344E+02,
		 0.127E+02, 0.191E+02,-0.322E+02,
		 0.142E+02, 0.210E+02,-0.299E+02,
		 0.159E+02, 0.234E+02,-0.277E+02,
		 0.176E+02, 0.258E+02,-0.247E+02,

		/* lad_43, starts at 21, does not bifurcate */
		 0.128E+02, 0.127E+02,-0.352E+02,
		 0.147E+02, 0.119E+02,-0.353E+02,
		 0.172E+02, 0.109E+02,-0.349E+02,
		 0.195E+02, 0.102E+02,-0.343E+02,
		 0.215E+02, 0.972E+01,-0.334E+02,
		 0.235E+02, 0.805E+01,-0.327E+02,
		 0.257E+02, 0.631E+01,-0.313E+02,
		 0.276E+02, 0.488E+01,-0.308E+02,
		 0.296E+02, 0.369E+01,-0.301E+02,
		 0.311E+02, 0.231E+01,-0.287E+02,
		 0.328E+02, 0.875E+00,-0.280E+02,
		 0.346E+02,-0.473E+00,-0.267E+02,
		 0.358E+02,-0.242E+01,-0.255E+02,

		/* lad_44, starts at 22, bifurcates at 28 */
		 0.320E+02, 0.137E+02,-0.250E+02,
		 0.338E+02, 0.143E+02,-0.230E+02,
		 0.349E+02, 0.154E+02,-0.210E+02,

		/* bifurcation 28, starts as lad_44, splits into lad_45 and lad_46 */
		 0.361E+02, 0.159E+02,-0.198E+02,

		/* lad_45, starts at 28, does not bifurcate */
		 0.363E+02, 0.157E+02,-0.191E+02,
		 0.378E+02, 0.157E+02,-0.175E+02,
		 0.397E+02, 0.158E+02,-0.151E+02,
		 0.408E+02, 0.172E+02,-0.131E+02,
		 0.407E+02, 0.186E+02,-0.109E+02,
		 0.404E+02, 0.195E+02,-0.921E+01,
		 0.396E+02, 0.205E+02,-0.683E+01,
		 0.393E+02, 0.218E+02,-0.547E+01,
		 0.386E+02, 0.223E+02,-0.395E+01,
		 0.382E+02, 0.232E+02,-0.214E+01,
		 0.374E+02, 0.236E+02, 0.133E+00,
		 0.366E+02, 0.245E+02, 0.236E+01,

		/* lad_46, starts at 28, does not bifurcate */
		 0.359E+02, 0.177E+02,-0.181E+02,
		 0.352E+02, 0.193E+02,-0.167E+02,
		 0.343E+02, 0.213E+02,-0.153E+02,
		 0.330E+02, 0.230E+02,-0.138E+02,
		 0.324E+02, 0.241E+02,-0.116E+02,
		 0.317E+02, 0.259E+02,-0.988E+01,
		 0.314E+02, 0.269E+02,-0.777E+01,
		 0.295E+02, 0.282E+02,-0.563E+01,

		/* lad_5, starts at 1, bifurcates at 3 */
		-0.331E+02,-0.745E+01,-0.165E+02,
		-0.331E+02,-0.103E+02,-0.160E+02,
		-0.320E+02,-0.130E+02,-0.160E+02,
		-0.308E+02,-0.156E+02,-0.157E+02,
		-0.300E+02,-0.183E+02,-0.154E+02,
		-0.287E+02,-0.200E+02,-0.149E+02,
		-0.274E+02,-0.230E+02,-0.141E+02,

		/* bifurcation 3, starts as lad_5, splits into lad_6 and lad_7 */
		-0.259E+02,-0.261E+02,-0.125E+02,

		/* lad_6, starts at 3, bifurcates at 5 */
		-0.245E+02,-0.282E+02,-0.105E+02,
		-0.222E+02,-0.308E+02,-0.720E+01,

		/* bifurcation 5, starts as lad_6, splits into lad_8 and lad_34 */
		-0.207E+02,-0.327E+02,-0.387E+01,

		/* lad_8, starts at 5, bifurcates at 7 */
		-0.182E+02,-0.334E+02, 0.120E+01,
		-0.170E+02,-0.329E+02, 0.583E+01,

		/* bifurcation 7, starts as lad_8, splits into lad_10 and lad_22 */
		-0.169E+02,-0.311E+02, 0.101E+02,

		/* lad_10, starts at 7, bifurcates at 11 */
		-0.165E+02,-0.294E+02, 0.139E+02,
		-0.158E+02,-0.271E+02, 0.172E+02,
		-0.149E+02,-0.245E+02, 0.200E+02,
		-0.159E+02,-0.215E+02, 0.220E+02,

		/* bifurcation 11, starts as lad_10, splits into lad_13 and lad_35 */
		-0.168E+02,-0.179E+02, 0.244E+02,

		/* lad_13, starts at 11, does not bifurcate */
		-0.182E+02,-0.150E+02, 0.251E+02,
		-0.187E+02,-0.119E+02, 0.254E+02,
		-0.187E+02,-0.913E+01, 0.269E+02,
		-0.187E+02,-0.678E+01, 0.281E+02,
		-0.190E+02,-0.450E+01, 0.278E+02,
		-0.186E+02,-0.324E+01, 0.289E+02,
		-0.170E+02,-0.229E+01, 0.297E+02,
		-0.142E+02,-0.111E+01, 0.314E+02,
		-0.118E+02,-0.112E+01, 0.319E+02,
		-0.900E+01,-0.687E+00, 0.322E+02,
		-0.660E+01,-0.243E+00, 0.326E+02,
		-0.440E+01, 0.667E+00, 0.327E+02,
		-0.210E+01, 0.114E+01, 0.332E+02,
		 0.500E+00, 0.205E+01, 0.329E+02,
		 0.260E+01, 0.252E+01, 0.330E+02,
		 0.570E+01, 0.275E+01, 0.331E+02,
		 0.780E+01, 0.297E+01, 0.329E+02,
		 0.100E+02, 0.294E+01, 0.326E+02,
		 0.121E+02, 0.331E+01, 0.317E+02,
		 0.141E+02, 0.346E+01, 0.311E+02,
		 0.166E+02, 0.367E+01, 0.310E+02,
		 0.187E+02, 0.405E+01, 0.305E+02,
		 0.208E+02, 0.459E+01, 0.298E+02,
		 0.228E+02, 0.467E+01, 0.290E+02,
		 0.257E+02, 0.456E+01, 0.283E+02,

		/* lad_7, starts at 3, bifurcates at 6 */
		-0.248E+02,-0.272E+02,-0.128E+02,
		-0.222E+02,-0.291E+02,-0.145E+02,
		-0.200E+02,-0.306E+02,-0.147E+02,
		-0.179E+02,-0.314E+02,-0.145E+02,
		-0.162E+02,-0.321E+02,-0.148E+02,
		-0.141E+02,-0.324E+02,-0.150E+02,
		-0.116E+02,-0.328E+02,-0.157E+02,
		-0.930E+01,-0.331E+02,-0.161E+02,
		-0.730E+01,-0.330E+02,-0.175E+02,

		/* bifurcation 6, starts as lad_7, splits into lad_9 and lad_19 */
		-0.580E+01,-0.329E+02,-0.181E+02,

		/* lad_9, starts at 6, bifurcates at 8 */
		-0.530E+01,-0.328E+02,-0.186E+02,
		-0.330E+01,-0.321E+02,-0.201E+02,
		-0.200E+00,-0.314E+02,-0.218E+02,
		 0.200E+01,-0.307E+02,-0.226E+02,

		/* bifurcation 8, starts as lad_9, splits into lad_11 and lad_15 */
		 0.460E+01,-0.299E+02,-0.237E+02,

		/* lad_11, starts at 8, does not bifurcate */
		 0.680E+01,-0.283E+02,-0.255E+02,
		 0.910E+01,-0.266E+02,-0.268E+02,
		 0.117E+02,-0.253E+02,-0.273E+02,
		 0.139E+02,-0.238E+02,-0.277E+02,
		 0.159E+02,-0.222E+02,-0.281E+02,
		 0.179E+02,-0.204E+02,-0.286E+02,
		 0.209E+02,-0.193E+02,-0.282E+02,
		 0.229E+02,-0.180E+02,-0.276E+02,
		 0.257E+02,-0.176E+02,-0.258E+02,
		 0.280E+02,-0.173E+02,-0.249E+02,
		 0.298E+02,-0.168E+02,-0.242E+02,

		/* lad_15, starts at 8, bifurcates at 14 */
		 0.640E+01,-0.299E+02,-0.231E+02,
		 0.860E+01,-0.297E+02,-0.229E+02,
		 0.117E+02,-0.298E+02,-0.220E+02,

		/* bifurcation 14, starts as lad_15, splits into lad_16 and lad_17 */
		 0.145E+02,-0.289E+02,-0.213E+02,

		/* lad_16, starts at 14, does not bifurcate */
		 0.176E+02,-0.270E+02,-0.221E+02,
		 0.203E+02,-0.260E+02,-0.218E+02,
		 0.230E+02,-0.250E+02,-0.210E+02,
		 0.260E+02,-0.240E+02,-0.199E+02,
		 0.288E+02,-0.236E+02,-0.182E+02,
		 0.306E+02,-0.234E+02,-0.160E+02,
		 0.336E+02,-0.226E+02,-0.133E+02,

		/* lad_17, starts at 14, does not bifurcate */
		 0.165E+02,-0.294E+02,-0.196E+02,
		 0.183E+02,-0.297E+02,-0.172E+02,
		 0.205E+02,-0.298E+02,-0.148E+02,
		 0.228E+02,-0.293E+02,-0.130E+02,
		 0.258E+02,-0.277E+02,-0.116E+02,
		 0.286E+02,-0.269E+02,-0.100E+02,
		 0.303E+02,-0.262E+02,-0.888E+01,

		/* lad_20, starts at 15, does not bifurcate */
		 0.194E+02,-0.319E+02,-0.400E+01,
		 0.214E+02,-0.308E+02,-0.320E+01,
		 0.239E+02,-0.298E+02,-0.163E+01,
		 0.263E+02,-0.283E+02,-0.551E+00,
		 0.284E+02,-0.268E+02, 0.795E+00,

		/* lad_21, starts at 15, does not bifurcate */
		 0.180E+02,-0.322E+02,-0.244E+01,
		 0.185E+02,-0.317E+02,-0.186E+00,
		 0.193E+02,-0.307E+02, 0.176E+01,
		 0.205E+02,-0.303E+02, 0.365E+01,

		/* bifurcation 15, starts as lad_19, splits into lad_20 and lad_21 */
		 0.172E+02,-0.326E+02,-0.432E+01,

		/* lad_19, starts at 6, bifurcates at 15 */
		-0.390E+01,-0.333E+02,-0.168E+02,
		-0.240E+01,-0.335E+02,-0.158E+02,
		-0.400E+00,-0.337E+02,-0.150E+02,
		 0.140E+01,-0.338E+02,-0.145E+02,
		 0.320E+01,-0.342E+02,-0.132E+02,
		 0.490E+01,-0.341E+02,-0.124E+02,
		 0.720E+01,-0.340E+02,-0.110E+02,
		 0.940E+01,-0.339E+02,-0.105E+02,
		 0.114E+02,-0.338E+02,-0.891E+01,
		 0.135E+02,-0.332E+02,-0.752E+01,
		 0.153E+02,-0.329E+02,-0.601E+01,

		/* lad_22, starts at 7, bifurcates at 17 */
		-0.159E+02,-0.298E+02, 0.133E+02,
		-0.140E+02,-0.284E+02, 0.157E+02,
		-0.126E+02,-0.271E+02, 0.178E+02,
		-0.117E+02,-0.255E+02, 0.202E+02,
		-0.102E+02,-0.244E+02, 0.218E+02,
		-0.840E+01,-0.234E+02, 0.233E+02,
		-0.600E+01,-0.232E+02, 0.238E+02,
		-0.390E+01,-0.232E+02, 0.241E+02,

		/* bifurcation 17, starts as lad_22, splits into lad_23 and lad_30 */
		-0.110E+01,-0.242E+02, 0.235E+02,

		/* lad_23, starts at 17, bifurcates at 19 */
		 0.210E+01,-0.232E+02, 0.234E+02,
		 0.560E+01,-0.225E+02, 0.234E+02,
		 0.880E+01,-0.224E+02, 0.226E+02,
		 0.120E+02,-0.219E+02, 0.215E+02,
		 0.152E+02,-0.211E+02, 0.208E+02,
		 0.175E+02,-0.201E+02, 0.203E+02,
		 0.204E+02,-0.185E+02, 0.201E+02,

		/* bifurcation 19, starts as lad_23, splits into lad_24 and lad_37 */
		 0.231E+02,-0.168E+02, 0.203E+02,

		/* lad_24, starts at 19, does not bifurcate */
		 0.275E+02,-0.166E+02, 0.182E+02,
		 0.309E+02,-0.148E+02, 0.180E+02,
		 0.342E+02,-0.134E+02, 0.174E+02,
		 0.375E+02,-0.121E+02, 0.163E+02,
		 0.397E+02,-0.103E+02, 0.166E+02,
		 0.419E+02,-0.798E+01, 0.168E+02,
		 0.441E+02,-0.463E+01, 0.172E+02,
		 0.450E+02,-0.244E+01, 0.175E+02,
		 0.460E+02,-0.228E+00, 0.172E+02,
		 0.469E+02, 0.252E+01, 0.163E+02,
		 0.473E+02, 0.493E+01, 0.152E+02,

		/* lad_30, starts at 17, bifurcates at 26 */
		-0.300E+00,-0.254E+02, 0.219E+02,
		 0.130E+01,-0.260E+02, 0.210E+02,
		 0.300E+01,-0.266E+02, 0.196E+02,
		 0.580E+01,-0.268E+02, 0.187E+02,
		 0.900E+01,-0.274E+02, 0.175E+02,
		 0.125E+02,-0.272E+02, 0.157E+02,
		 0.150E+02,-0.268E+02, 0.150E+02,
		 0.176E+02,-0.265E+02, 0.139E+02,
		 0.198E+02,-0.262E+02, 0.124E+02,
		 0.220E+02,-0.258E+02, 0.117E+02,
		 0.244E+02,-0.254E+02, 0.107E+02,
		 0.264E+02,-0.249E+02, 0.894E+01,

		/* lad_47, starts at 26, does not bifurcate */
		 0.308E+02,-0.225E+02, 0.861E+01,
		 0.336E+02,-0.207E+02, 0.845E+01,
		 0.353E+02,-0.201E+02, 0.850E+01,
		 0.374E+02,-0.188E+02, 0.813E+01,
		 0.385E+02,-0.188E+02, 0.706E+01,

		/* bifurcation 26, starts as lad_30, splits into lad_31 and lad_47 */
		 0.277E+02,-0.246E+02, 0.825E+01,

		/* lad_31, starts at 26, bifurcates at 27 */
		 0.292E+02,-0.242E+02, 0.663E+01,
		 0.308E+02,-0.240E+02, 0.533E+01,
		 0.326E+02,-0.237E+02, 0.389E+01,
		 0.342E+02,-0.230E+02, 0.264E+01,
		 0.362E+02,-0.221E+02, 0.191E+01,
		 0.389E+02,-0.207E+02, 0.926E+00,
		 0.410E+02,-0.191E+02, 0.591E+00,
		 0.421E+02,-0.181E+02, 0.312E+00,
		 0.435E+02,-0.171E+02, 0.539E+00,
		 0.452E+02,-0.156E+02, 0.831E+00,

		/* bifurcation 27, starts as lad_31, splits into lad_32 and lad_33 */
		 0.469E+02,-0.136E+02, 0.673E-01,

		/* lad_32, starts at 27, does not bifurcate */
		 0.480E+02,-0.129E+02,-0.476E+00,
		 0.493E+02,-0.108E+02,-0.144E+01,

		/* lad_33, starts at 27, does not bifurcate */
		 0.469E+02,-0.140E+02,-0.121E+01,
		 0.467E+02,-0.144E+02,-0.354E+01,
		 0.465E+02,-0.138E+02,-0.621E+01,
		 0.463E+02,-0.139E+02,-0.853E+01,

		/* lad_37, starts at 19, does not bifurcate */
		 0.242E+02,-0.152E+02, 0.208E+02,
		 0.258E+02,-0.126E+02, 0.222E+02,
		 0.278E+02,-0.104E+02, 0.225E+02,
		 0.296E+02,-0.808E+01, 0.228E+02,
		 0.313E+02,-0.616E+01, 0.234E+02,
		 0.330E+02,-0.342E+01, 0.232E+02,
		 0.345E+02,-0.980E+00, 0.235E+02,
		 0.352E+02, 0.180E+01, 0.234E+02,
		 0.366E+02, 0.422E+01, 0.231E+02,
		 0.379E+02, 0.685E+01, 0.228E+02,
		 0.387E+02, 0.903E+01, 0.220E+02,
		 0.404E+02, 0.106E+02, 0.207E+02,

		/* lad_34, starts at 5, does not bifurcate */
		-0.176E+02,-0.342E+02,-0.927E+00,
		-0.155E+02,-0.342E+02, 0.270E+00,
		-0.142E+02,-0.345E+02, 0.148E+01,
		-0.119E+02,-0.348E+02, 0.222E+01,
		-0.960E+01,-0.349E+02, 0.297E+01,
		-0.660E+01,-0.354E+02, 0.326E+01,
		-0.420E+01,-0.353E+02, 0.325E+01,
		-0.210E+01,-0.356E+02, 0.327E+01,
		-0.200E+00,-0.355E+02, 0.251E+01,
		 0.120E+01,-0.355E+02, 0.127E+01,
		 0.310E+01,-0.354E+02, 0.290E-01,
		 0.500E+01,-0.352E+02,-0.462E+00,
		 0.710E+01,-0.349E+02,-0.119E+01,
		 0.950E+01,-0.345E+02,-0.118E+01,
		 0.116E+02,-0.340E+02,-0.116E+01,

		/* lad_35, starts at 11, does not bifurcate */
		-0.164E+02,-0.168E+02, 0.253E+02,
		-0.159E+02,-0.153E+02, 0.262E+02,
		-0.139E+02,-0.133E+02, 0.278E+02,
		-0.124E+02,-0.116E+02, 0.293E+02,
		-0.102E+02,-0.103E+02, 0.297E+02,
		-0.820E+01,-0.992E+01, 0.299E+02,
		-0.590E+01,-0.957E+01, 0.302E+02,
		-0.340E+01,-0.861E+01, 0.301E+02,
		-0.150E+01,-0.912E+01, 0.302E+02,
		 0.100E+01,-0.843E+01, 0.302E+02,
		 0.370E+01,-0.829E+01, 0.297E+02,
		 0.620E+01,-0.757E+01, 0.295E+02,
		 0.890E+01,-0.808E+01, 0.290E+02,
		 0.124E+02,-0.707E+01, 0.284E+02,

		/* pda_1, no starting bifurcation, bifurcates at 4 */
		-0.363E+02, 0.124E+02,-0.179E+02,

		/* bifurcation 4, starts as pda_1, splits into pda_2 and pda_5 */
		-0.363E+02, 0.152E+02,-0.174E+02,

		/* pda_2, starts at 4, bifurcates at 10 */
		-0.354E+02, 0.173E+02,-0.158E+02,
		-0.354E+02, 0.192E+02,-0.137E+02,

		/* bifurcation 10, starts as pda_2, splits into pda_3 and pda_6 */
		-0.359E+02, 0.208E+02,-0.126E+02,

		/* pda_3, starts at 10, bifurcates at 13 */
		-0.359E+02, 0.227E+02,-0.107E+02,
		-0.354E+02, 0.247E+02,-0.962E+01,
		-0.354E+02, 0.263E+02,-0.718E+01,
		-0.354E+02, 0.279E+02,-0.518E+01,
		-0.354E+02, 0.298E+02,-0.341E+01,
		-0.354E+02, 0.323E+02,-0.233E+01,
		-0.354E+02, 0.352E+02,-0.157E+01,
		-0.343E+02, 0.379E+02, 0.951E+00,

		/* bifurcation 13, starts as pda_3, splits into pda_4 and pda_7 */
		-0.341E+02, 0.395E+02, 0.347E+01,

		/* pda_4, starts at 13, does not bifurcate */
		-0.334E+02, 0.410E+02, 0.592E+01,
		-0.321E+02, 0.417E+02, 0.842E+01,
		-0.318E+02, 0.391E+02, 0.126E+02,
		-0.307E+02, 0.380E+02, 0.155E+02,
		-0.290E+02, 0.359E+02, 0.199E+02,
		-0.283E+02, 0.330E+02, 0.221E+02,
		-0.275E+02, 0.307E+02, 0.238E+02,
		-0.269E+02, 0.276E+02, 0.250E+02,
		-0.270E+02, 0.242E+02, 0.266E+02,
		-0.279E+02, 0.203E+02, 0.285E+02,

		/* pda_5, starts at 4, does not bifurcate */
		-0.355E+02, 0.146E+02,-0.195E+02,
		-0.355E+02, 0.146E+02,-0.221E+02,
		-0.355E+02, 0.132E+02,-0.254E+02,
		-0.355E+02, 0.122E+02,-0.287E+02,
		-0.358E+02, 0.111E+02,-0.322E+02,
		-0.367E+02, 0.103E+02,-0.354E+02,
		-0.363E+02, 0.105E+02,-0.380E+02,
		-0.363E+02, 0.108E+02,-0.405E+02,
		-0.350E+02, 0.112E+02,-0.429E+02,
		-0.339E+02, 0.112E+02,-0.441E+02,
		-0.324E+02, 0.115E+02,-0.454E+02,

		/* pda_6, starts at 10, does not bifurcate */
		-0.358E+02, 0.209E+02,-0.127E+02,
		-0.350E+02, 0.228E+02,-0.143E+02,
		-0.338E+02, 0.240E+02,-0.163E+02,
		-0.326E+02, 0.257E+02,-0.171E+02,
		-0.318E+02, 0.272E+02,-0.202E+02,
		-0.310E+02, 0.288E+02,-0.226E+02,
		-0.302E+02, 0.304E+02,-0.253E+02,
		-0.288E+02, 0.317E+02,-0.279E+02,
		-0.271E+02, 0.334E+02,-0.303E+02,
		-0.246E+02, 0.348E+02,-0.320E+02,
		-0.227E+02, 0.350E+02,-0.335E+02,
		-0.218E+02, 0.348E+02,-0.338E+02,

		/* pda_7, starts at 13, bifurcates at 18 */
		-0.317E+02, 0.426E+02, 0.584E+01,
		-0.285E+02, 0.436E+02, 0.849E+01,
		-0.252E+02, 0.444E+02, 0.961E+01,

		/* bifurcation 18, starts as pda_7, splits into pda_8 and pda_13 */
		-0.224E+02, 0.446E+02, 0.932E+01,

		/* pda_8, starts at 18, bifurcates at 23 */
		-0.200E+02, 0.445E+02, 0.994E+01,
		-0.178E+02, 0.447E+02, 0.110E+02,

		/* bifurcation 23, starts as pda_8, splits into pda_9 and pda_12 */
		-0.159E+02, 0.447E+02, 0.130E+02,

		/* pda_9, starts at 23, bifurcates at 25 */
		-0.138E+02, 0.445E+02, 0.136E+02,
		-0.106E+02, 0.440E+02, 0.148E+02,
		-0.750E+01, 0.440E+02, 0.155E+02,
		-0.530E+01, 0.437E+02, 0.161E+02,
		-0.300E+01, 0.435E+02, 0.167E+02,
		-0.100E+01, 0.431E+02, 0.165E+02,

		/* bifurcation 25, starts as pda_9, splits into pda_10 and pda_11 */
		 0.150E+01, 0.427E+02, 0.167E+02,

		/* pda_10, starts at 25, does not bifurcate */
		 0.260E+01, 0.423E+02, 0.169E+02,
		 0.380E+01, 0.414E+02, 0.176E+02,
		 0.570E+01, 0.408E+02, 0.187E+02,
		 0.780E+01, 0.397E+02, 0.195E+02,
		 0.103E+02, 0.392E+02, 0.190E+02,
		 0.128E+02, 0.388E+02, 0.181E+02,
		 0.149E+02, 0.383E+02, 0.172E+02,
		 0.172E+02, 0.372E+02, 0.167E+02,
		 0.195E+02, 0.358E+02, 0.158E+02,
		 0.216E+02, 0.350E+02, 0.157E+02,

		/* pda_11, starts at 25, does not bifurcate */
		 0.370E+01, 0.429E+02, 0.144E+02,
		 0.550E+01, 0.434E+02, 0.129E+02,
		 0.680E+01, 0.440E+02, 0.105E+02,
		 0.870E+01, 0.439E+02, 0.885E+01,
		 0.106E+02, 0.438E+02, 0.695E+01,
		 0.118E+02, 0.435E+02, 0.474E+01,

		/* pda_12, starts at 23, does not bifurcate */
		-0.149E+02, 0.446E+02, 0.130E+02,
		-0.140E+02, 0.434E+02, 0.156E+02,
		-0.132E+02, 0.427E+02, 0.167E+02,
		-0.123E+02, 0.421E+02, 0.182E+02,
		-0.114E+02, 0.411E+02, 0.199E+02,
		-0.103E+02, 0.393E+02, 0.217E+02,
		-0.930E+01, 0.380E+02, 0.235E+02,

		/* pda_13, starts at 18, bifurcates at 29 */
		-0.209E+02, 0.451E+02, 0.909E+01,
		-0.187E+02, 0.460E+02, 0.696E+01,
		-0.164E+02, 0.465E+02, 0.539E+01,

		/* bifurcation 29, starts as pda_13, splits into pda_14 and pda_15 */
		-0.147E+02, 0.472E+02, 0.513E+01,

		/* pda_14, starts at 29, does not bifurcate */
		-0.143E+02, 0.465E+02, 0.441E+01,
		-0.120E+02, 0.476E+02, 0.383E+01,
		-0.970E+01, 0.479E+02, 0.352E+01,
		-0.810E+01, 0.480E+02, 0.152E+01,
		-0.600E+01, 0.483E+02, 0.119E+01,
		-0.380E+01, 0.483E+02, 0.513E+00,
		-0.150E+01, 0.479E+02, 0.510E+00,
		 0.700E+00, 0.478E+02, 0.175E+00,
		 0.290E+01, 0.475E+02,-0.820E+00,
		 0.580E+01, 0.466E+02,-0.243E+01,
		 0.780E+01, 0.455E+02,-0.396E+01,
		 0.980E+01, 0.439E+02,-0.630E+01,
		 0.109E+02, 0.410E+02,-0.104E+02,
		 0.110E+02, 0.398E+02,-0.131E+02,

		/* pda_15, starts at 29, does not bifurcate */
		-0.139E+02, 0.474E+02, 0.282E+01,
		-0.134E+02, 0.475E+02, 0.506E+00,
		-0.130E+02, 0.481E+02,-0.217E+01,
		-0.122E+02, 0.484E+02,-0.490E+01,
		-0.120E+02, 0.479E+02,-0.826E+01,
		-0.116E+02, 0.477E+02,-0.117E+02,
		-0.112E+02, 0.471E+02,-0.144E+02,
		-0.108E+02, 0.467E+02,-0.172E+02,
		-0.104E+02, 0.455E+02,-0.200E+02,
		-0.102E+02, 0.440E+02,-0.232E+02,
		-0.100E+02, 0.421E+02,-0.265E+02,
	};

/*
Global functions
----------------
*/
struct Map *create_Map(enum Map_type *map_type,enum Colour_option colour_option,
	enum Contours_option contours_option,enum Electrodes_option electrodes_option,
	enum Fibres_option fibres_option,enum Landmarks_option landmarks_option,
	enum Projection projection,enum Contour_thickness contour_thickness,
	struct Rig **rig_pointer,int *event_number_address,
	int *potential_time_address,int *datum_address)
/*******************************************************************************
LAST MODIFIED : 14 August 1994

DESCRIPTION :
This function allocates memory for a map and initializes the fields to the
specified values.  It returns a pointer to the created map if successful and
NULL if not successful.
==============================================================================*/
{
	struct Map *map;

	ENTER(create_Map);
	/* allocate memory */
	if (MYMALLOC(map,struct Map,1))
	{
		/* assign fields */
		map->type=map_type;
		map->event_number=event_number_address;
		map->potential_time=potential_time_address;
		map->datum=datum_address;
		map->colour_option=colour_option;
		map->contours_option=contours_option;
		map->electrodes_option=electrodes_option;
		map->fibres_option=fibres_option;
		map->landmarks_option=landmarks_option;
		map->projection=projection;
		map->contour_thickness=contour_thickness;
		map->undecided_accepted=0;
		map->rig_pointer=rig_pointer;
		/*??? calculate from rig ? */
		map->number_of_electrodes=0;
		map->electrodes=(struct Device **)NULL;
		map->electrode_x=(int *)NULL;
		map->electrode_y=(int *)NULL;
		map->electrode_drawn=(char *)NULL;
		map->number_of_auxiliary=0;
		map->auxiliary=(struct Device **)NULL;
		map->auxiliary_x=(int *)NULL;
		map->auxiliary_y=(int *)NULL;
		map->minimum_value=1;
		map->maximum_value=0;
		map->contour_minimum=1;
		map->contour_maximum=0;
		map->number_of_contours=2;
		map->contour_x=(short int *)NULL;
		map->contour_y=(short int *)NULL;
		map->pixel_values=(float *)NULL;
		map->print=0;
	}
	else
	{
		print_message(1,"create_Map.  Could not allocate map");
	}
	LEAVE;

	return (map);
} /* create_Map */

int destroy_Map(struct Map **map)
/*******************************************************************************
LAST MODIFIED : 13 August 1994

DESCRIPTION :
This function deallocates the memory asociated with fields of <**map> (except
the <rig_pointer> field), deallocates the memory for <**map> and sets <*map> to
NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Map);
	return_code=1;
	if (map&&(*map))
	{
		MYFREE((*map)->electrodes);
		MYFREE((*map)->electrode_x);
		MYFREE((*map)->electrode_y);
		MYFREE((*map)->electrode_drawn);
		MYFREE((*map)->auxiliary);
		MYFREE((*map)->auxiliary_x);
		MYFREE((*map)->auxiliary_y);
		MYFREE((*map)->contour_x);
		MYFREE((*map)->contour_y);
		MYFREE((*map)->pixel_values);
		MYFREE(*map);
	}
	LEAVE;

	return (return_code);
} /* destroy_Map */

int update_colour_map(struct Map *map)
/*******************************************************************************
LAST MODIFIED : 14 August 1994

DESCRIPTION :
Updates the colour map being used for map.
==============================================================================*/
{
	float contour_maximum,contour_minimum,maximum_value,minimum_value,theta;
	int cell_number,cell_range,end_cell,i,number_of_contours,return_code,
		start_cell;
	XColor colour,spectrum_rgb[MAX_SPECTRUM_COLOURS];

	ENTER (update_colour_map);
	if (map&&(map->type))
	{
		return_code=1;
		minimum_value=map->minimum_value;
		maximum_value=map->maximum_value;
		contour_minimum=map->contour_minimum;
		contour_maximum=map->contour_maximum;
		if (maximum_value==minimum_value)
		{
			start_cell=0;
			end_cell=number_of_spectrum_colours;
		}
		else
		{
			start_cell=(int)((contour_minimum-minimum_value)/
				(maximum_value-minimum_value)*(float)(number_of_spectrum_colours-1)+
				0.5);
			end_cell=(int)((contour_maximum-minimum_value)/
				(maximum_value-minimum_value)*(float)(number_of_spectrum_colours-1)+
				0.5);
		}
		cell_range=end_cell-start_cell;
		/* adjust the computer colour map for colour map */
		if (map->colour_option==SHOW_COLOUR)
		{
			switch (*(map->type))
			{
				case ACTIVATION:
				{
					/* create a spectrum from red (early) to blue (late) */
					for (i=0;i<=start_cell;i++)
					{
						spectrum_rgb[i].pixel=spectrum_pixels[i];
						spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
						spectrum_rgb[i].red=65535;
						spectrum_rgb[i].blue=0;
						spectrum_rgb[i].green=0;
					}
					for (i=start_cell+1;i<end_cell;i++)
					{
						spectrum_rgb[i].pixel=spectrum_pixels[i];
						spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
						theta=(float)(i-start_cell)/(float)(cell_range)*6;
						if (theta<2)
						{
							spectrum_rgb[i].red=65535;
							spectrum_rgb[i].blue=0;
							if (theta<1)
							{
								spectrum_rgb[i].green=
									(unsigned int)((theta*0.75)*65535);
							}
							else
							{
								spectrum_rgb[i].green=
									(unsigned int)((0.75+(theta-1)*0.25)*65535);
							}
						}
						else
						{
							if (theta<4)
							{
								spectrum_rgb[i].red=(unsigned int)((4-theta)*32767);
								spectrum_rgb[i].green=65535;
								spectrum_rgb[i].blue=
									(unsigned int)((theta-2)*32767);
							}
							else
							{
								spectrum_rgb[i].red=0;
								spectrum_rgb[i].blue=65535;
								if (theta<5)
								{
									spectrum_rgb[i].green=
										(unsigned int)((1-(theta-4)*0.25)*65535);
								}
								else
								{
									spectrum_rgb[i].green=
										(unsigned int)((0.75-(theta-5)*0.75)*65535);
								}
							}
						}
					}
					for (i=end_cell;i<number_of_spectrum_colours;i++)
					{
						spectrum_rgb[i].pixel=spectrum_pixels[i];
						spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
						spectrum_rgb[i].red=0;
						spectrum_rgb[i].blue=65535;
						spectrum_rgb[i].green=0;
					}
				} break;
				case POTENTIAL:
				{
					/* create a spectrum from blue (low) to red (high) */
					for (i=0;i<=start_cell;i++)
					{
						spectrum_rgb[i].pixel=spectrum_pixels[i];
						spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
						spectrum_rgb[i].red=0;
						spectrum_rgb[i].blue=65535;
						spectrum_rgb[i].green=0;
					}
					for (i=start_cell+1;i<end_cell;i++)
					{
						spectrum_rgb[i].pixel=spectrum_pixels[i];
						spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
						theta=(float)(end_cell-i)/(float)(cell_range)*6;
						if (theta<2)
						{
							spectrum_rgb[i].red=65535;
							spectrum_rgb[i].blue=0;
							if (theta<1)
							{
								spectrum_rgb[i].green=
									(unsigned int)((theta*0.75)*65535);
							}
							else
							{
								spectrum_rgb[i].green=
									(unsigned int)((0.75+(theta-1)*0.25)*65535);
							}
						}
						else
						{
							if (theta<4)
							{
								spectrum_rgb[i].red=(unsigned int)((4-theta)*32767);
								spectrum_rgb[i].green=65535;
								spectrum_rgb[i].blue=
									(unsigned int)((theta-2)*32767);
							}
							else
							{
								spectrum_rgb[i].red=0;
								spectrum_rgb[i].blue=65535;
								if (theta<5)
								{
									spectrum_rgb[i].green=
										(unsigned int)((1-(theta-4)*0.25)*65535);
								}
								else
								{
									spectrum_rgb[i].green=
										(unsigned int)((0.75-(theta-5)*0.75)*65535);
								}
							}
						}
					}
					for (i=end_cell;i<number_of_spectrum_colours;i++)
					{
						spectrum_rgb[i].pixel=spectrum_pixels[i];
						spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
						spectrum_rgb[i].red=65535;
						spectrum_rgb[i].blue=0;
						spectrum_rgb[i].green=0;
					}
				} break;
			}
			/* hide the map boundary */
			colour.pixel=user_settings.background_drawing_colour;
			XQueryColor(display,colour_map,&colour);
			colour.pixel=map_boundary_pixel;
			colour.flags=DoRed|DoGreen|DoBlue;
			XStoreColor(display,colour_map,&colour);
		}
		else
		{
			/* use background drawing colour for the whole spectrum */
			colour.pixel=user_settings.background_drawing_colour;
			XQueryColor(display,colour_map,&colour);
			for (i=0;i<number_of_spectrum_colours;i++)
			{
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				spectrum_rgb[i].red=colour.red;
				spectrum_rgb[i].blue=colour.blue;
				spectrum_rgb[i].green=colour.green;
			}
			/* show the map boundary */
			colour.pixel=user_settings.contour_colour;
			XQueryColor(display,colour_map,&colour);
			colour.pixel=map_boundary_pixel;
			colour.flags=DoRed|DoGreen|DoBlue;
			XStoreColor(display,colour_map,&colour);
		}
		/* adjust the computer colour map for contours */
		if (SHOW_CONTOURS==map->contours_option)
		{
			if ((VARIABLE_THICKNESS==map->contour_thickness)||!(map->pixel_values))
			{
				colour.pixel=user_settings.contour_colour;
				XQueryColor(display,colour_map,&colour);
				number_of_contours=map->number_of_contours;
				for (i=0;i<number_of_contours;i++)
				{
					cell_number=(int)(((contour_maximum*(float)i+contour_minimum*
						(float)(number_of_contours-1-i))/(float)(number_of_contours-1)-
						minimum_value)/(maximum_value-minimum_value)*
						(float)(number_of_spectrum_colours-1)+0.5);
					spectrum_rgb[cell_number].pixel=spectrum_pixels[cell_number];
					spectrum_rgb[cell_number].flags=DoRed|DoGreen|DoBlue;
					spectrum_rgb[cell_number].red=colour.red;
					spectrum_rgb[cell_number].blue=colour.blue;
					spectrum_rgb[cell_number].green=colour.green;
				}
			}
		}
		XStoreColors(display,colour_map,spectrum_rgb,number_of_spectrum_colours);
	}
	else
	{
		return_code=0;
		print_message(1,"update_colour_map.  Missing map");
	}
	LEAVE;

	return (return_code);
} /* update_colour_map */

int draw_map(struct Map *map,int recalculate_interpolation,
	struct Drawing *drawing)
/*******************************************************************************
LAST MODIFIED : 14 August 1994

DESCRIPTION :
This function draws the <map> in the <drawing>.  If <recalculate_interpolation>
then the interpolation functions are recalculated and the colours for the
pixels are recalculated.
???Would like to develop a "PostScript driver" for X.  To get an idea of whats
involved I'll put a PostScript switch into this routine so that it either draws
to the drawing or writes to a postscript file.
???DB.  Use XDrawSegments for contours ?
==============================================================================*/
{
	char *column_valid_u_and_v,draw_boundary,draw_contours,draw_contour_value,
		*electrode_drawn,*first,interpolation_recalculated,*name,*row_valid_u_and_v,
		undecided_accepted,valid_mu_and_theta,valid_u_and_v,value_string[11];
	enum Map_type map_type;
	enum Rig_type rig_type;
	float a,b,c,contour_maximum,contour_minimum,contour_step,cos_mu_hat,
		cos_theta_hat,d,det,*dfdx,dfdx_i_j,dfdx_i_jm1,dfdx_im1_j,dfdx_im1_jm1,
		*dfdy,dfdy_i_j,dfdy_i_jm1,dfdy_im1_j,dfdy_im1_jm1,dmudxi1,dmudxi2,
		dthetadxi1,dthetadxi2,dxdmu,dxdtheta,dydmu,dydtheta,*d2fdxdy,d2fdxdy_i_j,
		d2fdxdy_i_jm1,d2fdxdy_im1_j,d2fdxdy_im1_jm1,error_mu,error_theta,*f,focus,
		f_approx,fibre_angle,fibre_angle_1,fibre_angle_2,fibre_angle_3,
		fibre_angle_4,fibre_length,fibre_x,fibre_y,f_i_j,f_i_jm1,f_im1_j,f_im1_jm1,
		f_value,height,h01_u,h01_v,h02_u,h02_v,h11_u,h11_v,h12_u,h12_v,lambda,
		*landmark_point,max_region_height,max_region_width,max_f,maximum_value,
		*max_x,*max_y,min_f,minimum_value,*min_x,*min_y,mu,mu_1,mu_2,mu_3,mu_4,pi,
		pi_over_2,*pixel_value,range_f,sin_mu_hat,sin_theta_hat,stretch_x,stretch_y,
		theta,theta_1,theta_2,theta_3,theta_4,two_pi,u,v,width,*x,xi_1,xi_2,*x_item,
		*x_mesh,x_screen,x_screen_left,x_screen_step,*y,*y_item,*y_mesh,y_screen,
		y_screen_top,y_screen_step;
	GC *graphics_context;
	int ascent,boundary_type,cell_number,cell_range,column,datum,contour_area,
		contour_areas_in_x,contour_areas_in_y,contour_x_spacing,contour_y_spacing,
		descent,direction,drawing_height,drawing_width,end,event_number,
		fibre_iteration,fibre_spacing,i,i_j,i_jm1,im1_j,im1_jm1,j,k,l,name_length,
		next_contour_x,number_of_devices,number_of_columns,number_of_contour_areas,
		number_of_contours,number_of_electrodes,number_of_mesh_columns,
		number_of_mesh_rows,number_of_regions,number_of_rows,pixel_left,pixel_top,
		potential_time,region_number,return_code,row,screen_region_height,
		screen_region_width,*screen_x,*screen_y,start,*start_x,*start_y,
		string_length,temp_int,valid_i_j,valid_i_jm1,valid_im1_j,valid_im1_jm1,
		x_border,x_name_border,x_offset,x_pixel,x_separation,x_string,y_border,
		y_name_border,y_offset,y_pixel,y_separation,y_string;
	Pixel background_drawing_colour,pixel;
	short int *contour_x,*contour_y;
	struct Device **device,**electrode;
	struct Device_description *description;
	struct Event *event;
	struct Fibre_node *local_fibre_node_1,*local_fibre_node_2,*local_fibre_node_3,
		*local_fibre_node_4;
	struct Interpolation_function *function;
	struct Region *current_region;
	struct Region_list_item *region_item;
	struct Rig *rig;
	struct Signal *signal;
	union Position *position;
	XCharStruct bounds;
	XImage *map_image;

	ENTER(draw_map);
	return_code=1;
	/* check arguments */
	if (map&&drawing)
	{
		drawing_width=drawing->width;
		drawing_height=drawing->height;
		background_drawing_colour=user_settings.background_drawing_colour;
		if (map->type)
		{
			map_type= *(map->type);
		}
		else
		{
			map_type=NO_COLOUR_MAP;
		}
		switch (map_type)
		{
			case ACTIVATION:
			{
				undecided_accepted=map->undecided_accepted;
				if (map->event_number)
				{
					event_number= *(map->event_number);
				}
				else
				{
					print_message(1,"draw_map.  Missing event_number");
				}
				if (map->datum)
				{
					datum= *(map->datum);
				}
				else
				{
					print_message(1,"draw_map.  Missing datum");
				}
			} break;
			case POTENTIAL:
			{
				undecided_accepted=map->undecided_accepted;
				if (map->potential_time)
				{
					potential_time= *(map->potential_time);
				}
				else
				{
					print_message(1,"draw_map.  Missing potential_time");
				}
			} break;
		}
		/* clear the map drawing area (not needed for PostScript) */
		XPSFillRectangle(display,drawing->pixel_map,
			graphics_contexts.background_drawing_colour,
			0,0,drawing_width,drawing_height);
		if (map->rig_pointer)
		{
			if (rig= *(map->rig_pointer))
			{
				rig_type=rig->type;
				if (rig_type==SOCK)
				{
					pi_over_2=2*atan(1);
					pi=2*pi_over_2;
					two_pi=2*pi;
				}
				/* determine the number of map regions */
				if (current_region=get_Rig_current_region(rig))
				{
					number_of_regions=1;
				}
				else
				{
					number_of_regions=rig->number_of_regions;
				}
				/* count the electrodes */
				number_of_electrodes=0;
				device=rig->devices;
				number_of_devices=rig->number_of_devices;
				while (number_of_devices>0)
				{
					if (((description=(*device)->description)->type==ELECTRODE)&&
						(!current_region||(current_region==description->region)))
					{
						number_of_electrodes++;
					}
					device++;
					number_of_devices--;
				}
				map->number_of_electrodes=number_of_electrodes;
				if (number_of_electrodes>0)
				{
					if (MYMALLOC(x,float,number_of_electrodes)&&
						MYMALLOC(y,float,number_of_electrodes)&&
						MYMALLOC(map->electrodes,struct Device *,number_of_electrodes)&&
						MYMALLOC(map->electrode_x,int,number_of_electrodes)&&
						MYMALLOC(map->electrode_y,int,number_of_electrodes)&&
						MYMALLOC(map->electrode_drawn,char,number_of_electrodes)&&
						MYMALLOC(first,char,number_of_regions)&&
						MYMALLOC(max_x,float,number_of_regions)&&
						MYMALLOC(min_x,float,number_of_regions)&&
						MYMALLOC(max_y,float,number_of_regions)&&
						MYMALLOC(min_y,float,number_of_regions)&&
						MYMALLOC(start_x,int,number_of_regions)&&
						MYMALLOC(start_y,int,number_of_regions))
					{
						/* calculate the projections of the electrode positions and the
							x and y ranges */
						device=rig->devices;
						number_of_devices=rig->number_of_devices;
						x_item=x;
						y_item=y;
						electrode=map->electrodes;
						for (i=number_of_regions;i>0;)
						{
							i--;
							first[i]=1;
						}
						x_border=4;
						y_border=4;
						if (rig_type==SOCK)
						{
							focus=rig->properties.sock.focus;
						}
						while (number_of_devices>0)
						{
							if (((description=(*device)->description)->type==ELECTRODE)&&
								(!current_region||(current_region==description->region)))
							{
								/* add device */
								*electrode= *device;
								/* update border size */
/*								if (description->name)
								{
									XTextExtents(font,description->name,
										strlen(description->name),&direction,&ascent,&descent,
										&bounds);
									x_name_border=(bounds.lbearing+bounds.rbearing+1)/2;
									if (x_name_border>x_border)
									{
										x_border=x_name_border;
									}
									y_name_border=ascent+1+descent;
									if (y_name_border>y_border)
									{
										y_border=y_name_border;
									}
								}*/
								/* perform projection and update ranges */
								if (current_region)
								{
									region_number=0;
								}
								else
								{
									region_number=description->region->number;
								}
								position= &(description->properties.electrode.position);
								switch (rig_type)
								{
									case SOCK:
									{
										cartesian_to_prolate_spheroidal(position->sock.x,
											position->sock.y,position->sock.z,focus,&lambda,&mu,
											&theta);
										switch (map->projection)
										{
											case HAMMER:
											{
												Hammer_projection(mu,theta,x_item,y_item);
												if (first[region_number])
												{
													first[region_number]=0;
													max_x[region_number]=mu;
												}
												else
												{
													if (mu>max_x[region_number])
													{
														max_x[region_number]=mu;
													}
												}
											} break;
											case POLAR:
											{
												polar_projection(mu,theta,x_item,y_item);
												if (first[region_number])
												{
													first[region_number]=0;
													max_x[region_number]=mu;
												}
												else
												{
													if (mu>max_x[region_number])
													{
														max_x[region_number]=mu;
													}
												}
											} break;
										}
									} break;
									case PATCH:
									{
										*x_item=position->patch.x;
										*y_item=position->patch.y;
										if (first[region_number])
										{
											first[region_number]=0;
											min_x[region_number]= *x_item;
											min_y[region_number]= *y_item;
											max_x[region_number]= *x_item;
											max_y[region_number]= *y_item;
										}
										else
										{
											if (*x_item<min_x[region_number])
											{
												min_x[region_number]= *x_item;
											}
											else
											{
												if (*x_item>max_x[region_number])
												{
													max_x[region_number]= *x_item;
												}
											}
											if (*y_item<min_y[region_number])
											{
												min_y[region_number]= *y_item;
											}
											else
											{
												if (*y_item>max_y[region_number])
												{
													max_y[region_number]= *y_item;
												}
											}
										}
									} break;
								}
								electrode++;
								x_item++;
								y_item++;
							}
							device++;
							number_of_devices--;
						}
						/* divide the drawing area into regions */
						max_region_width=0;
						max_region_height=0;
						for (i=0;i<number_of_regions;i++)
						{
							switch (rig_type)
							{
								case SOCK:
								{
									switch (map->projection)
									{
										case HAMMER:
										{
											Hammer_projection(max_x[i],0,&a,max_y+i);
											min_x[i]= -1;
											max_x[i]=1;
											min_y[i]= -1;
											/*???DB. temp */
											max_y[i]=1;
										} break;
										case POLAR:
										{
											/*???DB.  temp */
											max_x[i]=pi;
											min_x[i]= -max_x[i];
											min_y[i]=min_x[i];
											max_y[i]=max_x[i];
										} break;
									}
								} break;
							}
							if (max_x[i]-min_x[i]>max_region_width)
							{
								max_region_width=max_x[i]-min_x[i];
							}
							if (max_y[i]-min_y[i]>max_region_height)
							{
								max_region_height=max_y[i]-min_y[i];
							}
						}
						screen_region_width=(int)sqrt((float)((drawing_width)*
							(drawing_height))*max_region_width/
							((float)number_of_regions*max_region_height));
						if (drawing_width<screen_region_width)
						{
							screen_region_width=drawing_width;
						}
						number_of_columns=(drawing_width)/screen_region_width;
						number_of_rows=(number_of_regions-1)/number_of_columns+1;
						/* equalize the columns */
						if ((number_of_regions>1)&&(number_of_columns>1)&&
							((i=number_of_rows*number_of_columns-number_of_regions-1)>0))
						{
							number_of_rows -= i/(number_of_columns-1);
						}
						/* make the regions fill the width and the height */
						screen_region_width=(drawing_width)/number_of_columns;
						screen_region_height=(drawing_height)/number_of_rows;
						/* calculate the transformation from map coordinates to screen
							coordinates */
						if ((max_region_width==0.0)||
							(screen_region_width<=2*x_border+1))
						{
							*start_x=(screen_region_width)/2;
							stretch_x=0;
						}
						else
						{
/*							*start_x=x_border+1;
							stretch_x=((float)(screen_region_width-2*(x_border+1)))/
								max_region_width;*/
							*start_x=x_border;
							stretch_x=((float)(screen_region_width-(2*x_border+1)))/
								max_region_width;
						}
/*						if ((max_region_height==0.0)||(screen_region_height<=y_border+5))*/
						/* allow room to write the electrode names */
						XTextExtents(font,"H",1,&direction,&ascent,&descent,&bounds);
						if ((max_region_height==0.0)||
							(screen_region_height<=2*y_border+ascent+descent+1))
						{
							*start_y=(screen_region_height)/2;
							stretch_y=0;
						}
						else
						{
/*							*start_y=screen_region_height-3;
							stretch_y=((float)(screen_region_height-(y_border+5)))/
								max_region_height;*/
							*start_y=screen_region_height-(y_border+1);
							stretch_y=((float)(screen_region_height-
								(2*y_border+ascent+descent+1)))/max_region_height;
						}
						for (i=1;i<number_of_regions;i++)
						{
							start_x[i]=start_x[0]+((i/number_of_rows)*drawing_width)/
								number_of_columns;
							start_y[i]=start_y[0]+((i%number_of_rows)*drawing_height)/
								number_of_rows;
						}
						/*??? draw grid */
						/* construct a colour map image for colour map or contours or
							values */
							/*???Only really need to calculate the interpolation function for
								for values ? */
						/* draw colour map and contours first (background) */
						if ((stretch_x!=0)&&(stretch_y!=0)&&(map_type!=NO_COLOUR_MAP))
						{
							if (map_image=drawing->image)
							{
								if (recalculate_interpolation)
								{
									/* allocate memory for drawing the map boundary and drawing
										contour values */
									contour_x_spacing=user_settings.pixels_between_contour_values;
									contour_areas_in_x=drawing_width/contour_x_spacing;
									contour_x_spacing=drawing_width/contour_areas_in_x;
									if (contour_x_spacing*contour_areas_in_x<drawing_width)
									{
										if ((contour_x_spacing+1)*(contour_areas_in_x-1)<
											(contour_x_spacing-1)*(contour_areas_in_x+1))
										{
											contour_x_spacing++;
										}
										else
										{
											contour_areas_in_x++;
										}
									}
									contour_y_spacing=user_settings.pixels_between_contour_values;
									contour_areas_in_y=drawing_height/contour_y_spacing;
									contour_y_spacing=drawing_height/contour_areas_in_y;
									if (contour_y_spacing*contour_areas_in_y<drawing_height)
									{
										if ((contour_y_spacing+1)*(contour_areas_in_y-1)<
											(contour_y_spacing-1)*(contour_areas_in_y+1))
										{
											contour_y_spacing++;
										}
										else
										{
											contour_areas_in_y++;
										}
									}
									number_of_contour_areas=contour_areas_in_x*contour_areas_in_y;
									if (MYMALLOC(row_valid_u_and_v,char,screen_region_width+1)&&
										MYMALLOC(contour_x,short int,number_of_contour_areas*
										number_of_spectrum_colours)&&MYMALLOC(contour_y,short int,
										number_of_contour_areas*number_of_spectrum_colours))
									{
										/* free memory for drawing contour values */
										MYFREE(map->contour_x);
										MYFREE(map->contour_y);
										map->number_of_contour_areas=number_of_contour_areas;
										map->number_of_contour_areas_in_x=contour_areas_in_x;
										map->contour_x=contour_x;
										map->contour_y=contour_y;
										/* initialize the contour areas */
										for (i=number_of_contour_areas*number_of_spectrum_colours;
											i>0;i--)
										{
											*contour_x= -1;
											*contour_y= -1;
											contour_x++;
											contour_y++;
										}
										busy_cursor_on((Widget)NULL);
										/* calculate the interpolation function(s) and determine the
											range of function values */
										if (MYREALLOC(pixel_value,map->pixel_values,float,
											drawing_width*drawing_height))
										{
											map->pixel_values=pixel_value;
											region_item=rig->region_list;
											interpolation_recalculated=0;
											for (region_number=0;region_number<number_of_regions;
												region_number++)
											{
												if (number_of_regions>1)
												{
													current_region=region_item->region;
												}
												else
												{
													current_region=get_Rig_current_region(rig);
												}
												/* interpolate data */
												if (!(current_region->interpolation_function))
												{
													interpolation_recalculated=1;
													current_region->interpolation_function=
														calculate_interpolation_functio(map_type,rig,
														current_region,map->event_number,
														map->potential_time,map->datum,undecided_accepted);
												}
												region_item=region_item->next;
											}
											min_f=1;
											max_f=0;
											pixel=spectrum_pixels[0];
											/* for each region */
											region_item=rig->region_list;
											for (region_number=0;region_number<number_of_regions;
												region_number++)
											{
												if (number_of_regions>1)
												{
													current_region=region_item->region;
												}
												else
												{
													current_region=get_Rig_current_region(rig);
												}
												/* interpolate data */
												if (function=current_region->interpolation_function)
												{
													f=function->f;
													dfdx=function->dfdx;
													dfdy=function->dfdy;
													d2fdxdy=function->d2fdxdy;
													number_of_mesh_rows=function->number_of_rows;
													number_of_mesh_columns=function->number_of_columns;
													y_mesh=function->y_mesh;
													x_mesh=function->x_mesh;
													/* draw colour map */
													column_valid_u_and_v=row_valid_u_and_v+1;
													for (i=screen_region_width;i>0;i--)
													{
														*column_valid_u_and_v=0;
														column_valid_u_and_v++;
													}
													pixel_left=((region_number/number_of_rows)*
														drawing_width)/number_of_columns;
													pixel_top=((region_number%number_of_rows)*
														drawing_height)/number_of_rows;
													x_screen_step=1/stretch_x;
													y_screen_step= -1/stretch_y;
													x_screen_left=min_x[region_number]+
														(pixel_left-start_x[region_number])*x_screen_step;
													y_screen_top=min_y[region_number]+
														(pixel_top-start_y[region_number])*y_screen_step;
													y_screen=y_screen_top;
													y_pixel=pixel_top;
													for (j=0;j<screen_region_height;j++)
													{
														x_screen=x_screen_left;
														x_pixel=pixel_left;
														column_valid_u_and_v=row_valid_u_and_v;
														*column_valid_u_and_v=0;
														for (i=0;i<screen_region_width;i++)
														{
															/* calculate the element coordinates */
															switch (rig_type)
															{
																case SOCK:
																{
																	switch (map->projection)
																	{
																		case HAMMER:
																		{
																			/*???avoid singularity ? */
																			if ((x_screen!=0)||
																				((y_screen!=1)&&(y_screen!= -1)))
																			{
																				a=x_screen*x_screen;
																				b=y_screen*y_screen;
																				c=2-a-b;
																				if (c>1)
																				{
																					d=y_screen*sqrt(c);
																					if (d>=1)
																					{
																						mu=pi;
																					}
																					else
																					{
																						if (d<= -1)
																						{
																							mu=0;
																						}
																						else
																						{
																							mu=pi_over_2+asin(d);
																						}
																					}
																					d=sqrt((a*c)/(1-b*c));
																					if (x_screen>0)
																					{
																						if (d>=1)
																						{
																							theta=two_pi;
																						}
																						else
																						{
																							theta=pi-2*asin(d);
																						}
																					}
																					else
																					{
																						if (d>=1)
																						{
																							theta=0;
																						}
																						else
																						{
																							theta=pi+2*asin(d);
																						}
																					}
																					u=theta;
																					v=mu;
																					valid_u_and_v=1;
																				}
																				else
																				{
																					valid_u_and_v=0;
																				}
																			}
																			else
																			{
																				valid_u_and_v=0;
																			}
																		} break;
																		case POLAR:
																		{
																			mu=sqrt(x_screen*x_screen+
																				y_screen*y_screen);
																			if (mu>0)
																			{
																				if (x_screen>0)
																				{
																					if (y_screen>0)
																					{
																						theta=asin(y_screen/mu);
																					}
																					else
																					{
																						theta=two_pi+asin(y_screen/mu);
																					}
																				}
																				else
																				{
																					theta=pi-asin(y_screen/mu);
																				}
																			}
																			else
																			{
																				theta=0;
																			}
																			u=theta;
																			v=mu;
																			valid_u_and_v=1;
																		} break;
																	}
																} break;
																case PATCH:
																{
																	u=x_screen;
																	v=y_screen;
																	valid_u_and_v=1;
																} break;
															}
															if (valid_u_and_v)
															{
																/* determine which element the point is in */
																column=0;
																while ((column<number_of_mesh_columns)&&
																	(u>=x_mesh[column]))
																{
																	column++;
																}
																if ((column>0)&&(u<=x_mesh[column]))
																{
																	row=0;
																	while ((row<number_of_mesh_rows)&&
																		(v>=y_mesh[row]))
																	{
																		row++;
																	}
																	if ((row>0)&&(v<=y_mesh[row]))
																	{
																		/* calculate basis function values */
																		width=x_mesh[column]-x_mesh[column-1];
																		h11_u=h12_u=u-x_mesh[column-1];
																		u=h11_u/width;
																		h01_u=(2*u-3)*u*u+1;
																		h11_u *= (u-1)*(u-1);
																		h02_u=u*u*(3-2*u);
																		h12_u *= u*(u-1);
																		height=y_mesh[row]-y_mesh[row-1];
																		h11_v=h12_v=v-y_mesh[row-1];
																		v=h11_v/height;
																		h01_v=(2*v-3)*v*v+1;
																		h11_v *= (v-1)*(v-1);
																		h02_v=v*v*(3-2*v);
																		h12_v *= v*(v-1);
																		/* calculate the interpolation function
																			coefficients */
																		f_im1_jm1=h01_u*h01_v;
																		f_im1_j=h02_u*h01_v;
																		f_i_j=h02_u*h02_v;
																		f_i_jm1=h01_u*h02_v;
																		dfdx_im1_jm1=h11_u*h01_v;
																		dfdx_im1_j=h12_u*h01_v;
																		dfdx_i_j=h12_u*h02_v;
																		dfdx_i_jm1=h11_u*h02_v;
																		dfdy_im1_jm1=h01_u*h11_v;
																		dfdy_im1_j=h02_u*h11_v;
																		dfdy_i_j=h02_u*h12_v;
																		dfdy_i_jm1=h01_u*h12_v;
																		d2fdxdy_im1_jm1=h11_u*h11_v;
																		d2fdxdy_im1_j=h12_u*h11_v;
																		d2fdxdy_i_j=h12_u*h12_v;
																		d2fdxdy_i_jm1=h11_u*h12_v;
																		/* calculate node numbers
																			NB the local coordinates have the top
																			right corner of the element as the origin.
																			This means that
																			(i-1,j-1) is the bottom left corner
																			(i,j-1) is the top left corner
																			(i,j) is the top right corner
																			(i-1,j) is the bottom right corner */
																		/* (i-1,j-1) node (bottom left corner) */
																		im1_jm1=(row-1)*(number_of_mesh_columns+1)+
																			column-1;
																		/* (i,j-1) node (top left corner) */
																		i_jm1=row*(number_of_mesh_columns+1)+
																			column-1;
																		/* (i,j) node (top right corner) */
																		i_j=row*(number_of_mesh_columns+1)+column;
																		/* (i-1,j) node (bottom right corner) */
																		im1_j=(row-1)*(number_of_mesh_columns+1)+
																			column;
																		f_approx=
																			f[im1_jm1]*f_im1_jm1+
																			f[i_jm1]*f_i_jm1+
																			f[i_j]*f_i_j+
																			f[im1_j]*f_im1_j+
																			dfdx[im1_jm1]*dfdx_im1_jm1+
																			dfdx[i_jm1]*dfdx_i_jm1+
																			dfdx[i_j]*dfdx_i_j+
																			dfdx[im1_j]*dfdx_im1_j+
																			dfdy[im1_jm1]*dfdy_im1_jm1+
																			dfdy[i_jm1]*dfdy_i_jm1+
																			dfdy[i_j]*dfdy_i_j+
																			dfdy[im1_j]*dfdy_im1_j+
																			d2fdxdy[im1_jm1]*d2fdxdy_im1_jm1+
																			d2fdxdy[i_jm1]*d2fdxdy_i_jm1+
																			d2fdxdy[i_j]*d2fdxdy_i_j+
																			d2fdxdy[im1_j]*d2fdxdy_im1_j;
																		if (max_f<min_f)
																		{
																			min_f=f_approx;
																			max_f=f_approx;
																		}
																		else
																		{
																			if (f_approx<min_f)
																			{
																				min_f=f_approx;
																			}
																			else
																			{
																				if (f_approx>max_f)
																				{
																					max_f=f_approx;
																				}
																			}
																		}
																		XPutPixel(map_image,x_pixel,y_pixel,pixel);
																		pixel_value[y_pixel*drawing_width+x_pixel]=
																			f_approx;
																		if (!(*(column_valid_u_and_v++))&&
																			(x_pixel>pixel_left))
																		{
																			XPutPixel(map_image,x_pixel-1,y_pixel,
																				map_boundary_pixel);
																		}
																		if (!(*(column_valid_u_and_v))&&
																			(y_pixel>pixel_top))
																		{
																			XPutPixel(map_image,x_pixel,y_pixel-1,
																				map_boundary_pixel);
																		}
																		*column_valid_u_and_v=1;
																	}
																	else
																	{
																		if (*(column_valid_u_and_v++)||
																			*column_valid_u_and_v)
																		{
																			XPutPixel(map_image,x_pixel,y_pixel,
																				map_boundary_pixel);
																		}
																		else
																		{
																			XPutPixel(map_image,x_pixel,y_pixel,
																				background_drawing_colour);
																		}
																		*column_valid_u_and_v=0;
																	}
																}
																else
																{
																	if (*(column_valid_u_and_v++)||
																		*column_valid_u_and_v)
																	{
																		XPutPixel(map_image,x_pixel,y_pixel,
																			map_boundary_pixel);
																	}
																	else
																	{
																		XPutPixel(map_image,x_pixel,y_pixel,
																			background_drawing_colour);
																	}
																	*column_valid_u_and_v=0;
																}
															}
															else
															{
																if (*(column_valid_u_and_v++)||
																	*column_valid_u_and_v)
																{
																	XPutPixel(map_image,x_pixel,y_pixel,
																		map_boundary_pixel);
																}
																else
																{
																	XPutPixel(map_image,x_pixel,y_pixel,
																		background_drawing_colour);
																}
																*column_valid_u_and_v=0;
															}
															x_screen += x_screen_step;
															x_pixel++;
														}
														y_screen += y_screen_step;
														y_pixel++;
													}
												}
												region_item=region_item->next;
											}
											if (interpolation_recalculated)
											{
												map->minimum_value=min_f;
												map->maximum_value=max_f;
												map->contour_minimum=min_f;
												map->contour_maximum=max_f;
											}
											else
											{
												min_f=map->minimum_value;
												max_f=map->maximum_value;
											}
											/* calculate range of values */
											range_f=max_f-min_f;
											if (range_f<=0)
											{
												range_f=1;
											}
											for (y_pixel=0;y_pixel<drawing_height;y_pixel++)
											{
												contour_area=
													(y_pixel/contour_y_spacing)*contour_areas_in_x;
												next_contour_x=contour_x_spacing-1;
												contour_x=(map->contour_x)+contour_area;
												contour_y=(map->contour_y)+contour_area;
												for (x_pixel=0;x_pixel<drawing_width;x_pixel++)
												{
													if (pixel==XGetPixel(map_image,x_pixel,y_pixel))
													{
														cell_number=(int)(((*pixel_value)-min_f)*
															(float)(number_of_spectrum_colours-1)/
															range_f+0.5);
														XPutPixel(map_image,x_pixel,y_pixel,
															spectrum_pixels[cell_number]);
														cell_number *= number_of_contour_areas;
														contour_x[cell_number]=x_pixel;
														contour_y[cell_number]=y_pixel;
													}
													pixel_value++;
													if (x_pixel>=next_contour_x)
													{
														contour_x++;
														contour_y++;
														next_contour_x += contour_x_spacing;
													}
												}
											}
										}
										else
										{
											/*???DB.  Ghost the constant thickness button ? */
											MYFREE(map->pixel_values);
											min_f=1;
											max_f=0;
											region_item=rig->region_list;
											interpolation_recalculated=0;
											for (region_number=0;region_number<number_of_regions;
												region_number++)
											{
												if (number_of_regions>1)
												{
													current_region=region_item->region;
												}
												else
												{
													current_region=get_Rig_current_region(rig);
												}
												/* interpolate data */
												if (!(function=current_region->interpolation_function))
												{
													interpolation_recalculated=1;
													function=current_region->interpolation_function=
														calculate_interpolation_functio(map_type,rig,
														current_region,map->event_number,
														map->potential_time,map->datum,undecided_accepted);
												}
												if (function)
												{
													if (min_f>max_f)
													{
														min_f=function->f_min;
														max_f=function->f_max;
													}
													else
													{
														if (function->f_min<min_f)
														{
															min_f=function->f_min;
														}
														if (max_f<function->f_max)
														{
															max_f=function->f_max;
														}
													}
												}
												region_item=region_item->next;
											}
											if (interpolation_recalculated)
											{
												map->minimum_value=min_f;
												map->maximum_value=max_f;
												map->contour_minimum=min_f;
												map->contour_maximum=max_f;
											}
											else
											{
												min_f=map->minimum_value;
												max_f=map->maximum_value;
											}
											/* calculate range of values */
											range_f=max_f-min_f;
											if (range_f<=0)
											{
												range_f=1;
											}
											/* for each region */
											region_item=rig->region_list;
											for (region_number=0;region_number<number_of_regions;
												region_number++)
											{
												if (number_of_regions>1)
												{
													current_region=region_item->region;
												}
												else
												{
													current_region=get_Rig_current_region(rig);
												}
												/* interpolate data */
												if (function=current_region->interpolation_function)
												{
													f=function->f;
													dfdx=function->dfdx;
													dfdy=function->dfdy;
													d2fdxdy=function->d2fdxdy;
													number_of_mesh_rows=function->number_of_rows;
													number_of_mesh_columns=function->number_of_columns;
													y_mesh=function->y_mesh;
													x_mesh=function->x_mesh;
													/* draw colour map */
													column_valid_u_and_v=row_valid_u_and_v+1;
													for (i=screen_region_width;i>0;i--)
													{
														*column_valid_u_and_v=0;
														column_valid_u_and_v++;
													}
													pixel_left=((region_number/number_of_rows)*
														drawing_width)/number_of_columns;
													pixel_top=((region_number%number_of_rows)*
														drawing_height)/number_of_rows;
													x_screen_step=1/stretch_x;
													y_screen_step= -1/stretch_y;
													x_screen_left=min_x[region_number]+
														(pixel_left-start_x[region_number])*x_screen_step;
													y_screen_top=min_y[region_number]+
														(pixel_top-start_y[region_number])*y_screen_step;
													y_screen=y_screen_top;
													y_pixel=pixel_top;
													for (j=0;j<screen_region_height;j++)
													{
														x_screen=x_screen_left;
														x_pixel=pixel_left;
														contour_area=
															(y_pixel/contour_y_spacing)*contour_areas_in_x+
															(x_pixel/contour_x_spacing);
														next_contour_x=((x_pixel/contour_x_spacing)+1)*
															contour_x_spacing;
														contour_x=(map->contour_x)+contour_area;
														contour_y=(map->contour_y)+contour_area;
														column_valid_u_and_v=row_valid_u_and_v;
														*column_valid_u_and_v=0;
														for (i=0;i<screen_region_width;i++)
														{
															/* calculate the element coordinates */
															switch (rig_type)
															{
																case SOCK:
																{
																	switch (map->projection)
																	{
																		case HAMMER:
																		{
																			/*???avoid singularity ? */
																			if ((x_screen!=0)||
																				((y_screen!=1)&&(y_screen!= -1)))
																			{
																				a=x_screen*x_screen;
																				b=y_screen*y_screen;
																				c=2-a-b;
																				if (c>1)
																				{
																					d=y_screen*sqrt(c);
																					if (d>=1)
																					{
																						mu=pi;
																					}
																					else
																					{
																						if (d<= -1)
																						{
																							mu=0;
																						}
																						else
																						{
																							mu=pi_over_2+asin(d);
																						}
																					}
																					d=sqrt((a*c)/(1-b*c));
																					if (x_screen>0)
																					{
																						if (d>=1)
																						{
																							theta=two_pi;
																						}
																						else
																						{
																							theta=pi-2*asin(d);
																						}
																					}
																					else
																					{
																						if (d>=1)
																						{
																							theta=0;
																						}
																						else
																						{
																							theta=pi+2*asin(d);
																						}
																					}
																					u=theta;
																					v=mu;
																					valid_u_and_v=1;
																				}
																				else
																				{
																					valid_u_and_v=0;
																				}
																			}
																			else
																			{
																				valid_u_and_v=0;
																			}
																		} break;
																		case POLAR:
																		{
																			mu=sqrt(x_screen*x_screen+
																				y_screen*y_screen);
																			if (mu>0)
																			{
																				if (x_screen>0)
																				{
																					if (y_screen>0)
																					{
																						theta=asin(y_screen/mu);
																					}
																					else
																					{
																						theta=two_pi+asin(y_screen/mu);
																					}
																				}
																				else
																				{
																					theta=pi-asin(y_screen/mu);
																				}
																			}
																			else
																			{
																				theta=0;
																			}
																			u=theta;
																			v=mu;
																			valid_u_and_v=1;
																		} break;
																	}
																} break;
																case PATCH:
																{
																	u=x_screen;
																	v=y_screen;
																	valid_u_and_v=1;
																} break;
															}
															if (valid_u_and_v)
															{
																/* determine which element the point is in */
																column=0;
																while ((column<number_of_mesh_columns)&&
																	(u>=x_mesh[column]))
																{
																	column++;
																}
																if ((column>0)&&(u<=x_mesh[column]))
																{
																	row=0;
																	while ((row<number_of_mesh_rows)&&
																		(v>=y_mesh[row]))
																	{
																		row++;
																	}
																	if ((row>0)&&(v<=y_mesh[row]))
																	{
																		/* calculate basis function values */
																		width=x_mesh[column]-x_mesh[column-1];
																		h11_u=h12_u=u-x_mesh[column-1];
																		u=h11_u/width;
																		h01_u=(2*u-3)*u*u+1;
																		h11_u *= (u-1)*(u-1);
																		h02_u=u*u*(3-2*u);
																		h12_u *= u*(u-1);
																		height=y_mesh[row]-y_mesh[row-1];
																		h11_v=h12_v=v-y_mesh[row-1];
																		v=h11_v/height;
																		h01_v=(2*v-3)*v*v+1;
																		h11_v *= (v-1)*(v-1);
																		h02_v=v*v*(3-2*v);
																		h12_v *= v*(v-1);
																		/* calculate the interpolation function
																			coefficients */
																		f_im1_jm1=h01_u*h01_v;
																		f_im1_j=h02_u*h01_v;
																		f_i_j=h02_u*h02_v;
																		f_i_jm1=h01_u*h02_v;
																		dfdx_im1_jm1=h11_u*h01_v;
																		dfdx_im1_j=h12_u*h01_v;
																		dfdx_i_j=h12_u*h02_v;
																		dfdx_i_jm1=h11_u*h02_v;
																		dfdy_im1_jm1=h01_u*h11_v;
																		dfdy_im1_j=h02_u*h11_v;
																		dfdy_i_j=h02_u*h12_v;
																		dfdy_i_jm1=h01_u*h12_v;
																		d2fdxdy_im1_jm1=h11_u*h11_v;
																		d2fdxdy_im1_j=h12_u*h11_v;
																		d2fdxdy_i_j=h12_u*h12_v;
																		d2fdxdy_i_jm1=h11_u*h12_v;
																		/* calculate node numbers
																			NB the local coordinates have the top
																			right corner of the element as the origin.
																			This means that
																			(i-1,j-1) is the bottom left corner
																			(i,j-1) is the top left corner
																			(i,j) is the top right corner
																			(i-1,j) is the bottom right corner */
																		/* (i-1,j-1) node (bottom left corner) */
																		im1_jm1=(row-1)*(number_of_mesh_columns+1)+
																			column-1;
																		/* (i,j-1) node (top left corner) */
																		i_jm1=row*(number_of_mesh_columns+1)+
																			column-1;
																		/* (i,j) node (top right corner) */
																		i_j=row*(number_of_mesh_columns+1)+column;
																		/* (i-1,j) node (bottom right corner) */
																		im1_j=(row-1)*(number_of_mesh_columns+1)+
																			column;
																		f_approx=
																			f[im1_jm1]*f_im1_jm1+
																			f[i_jm1]*f_i_jm1+
																			f[i_j]*f_i_j+
																			f[im1_j]*f_im1_j+
																			dfdx[im1_jm1]*dfdx_im1_jm1+
																			dfdx[i_jm1]*dfdx_i_jm1+
																			dfdx[i_j]*dfdx_i_j+
																			dfdx[im1_j]*dfdx_im1_j+
																			dfdy[im1_jm1]*dfdy_im1_jm1+
																			dfdy[i_jm1]*dfdy_i_jm1+
																			dfdy[i_j]*dfdy_i_j+
																			dfdy[im1_j]*dfdy_im1_j+
																			d2fdxdy[im1_jm1]*d2fdxdy_im1_jm1+
																			d2fdxdy[i_jm1]*d2fdxdy_i_jm1+
																			d2fdxdy[i_j]*d2fdxdy_i_j+
																			d2fdxdy[im1_j]*d2fdxdy_im1_j;
																		if (f_approx<min_f)
																		{
																			f_approx=min_f;
																		}
																		else
																		{
																			if (f_approx>max_f)
																			{
																				f_approx=max_f;
																			}
																		}
																		cell_number=(int)((f_approx-min_f)*
																			(float)(number_of_spectrum_colours-1)/
																			range_f+0.5);
																		XPutPixel(map_image,x_pixel,y_pixel,
																			spectrum_pixels[cell_number]);
																		cell_number *= number_of_contour_areas;
																		contour_x[cell_number]=x_pixel;
																		contour_y[cell_number]=y_pixel;
																		if (!(*(column_valid_u_and_v++))&&
																			(x_pixel>pixel_left))
																		{
																			XPutPixel(map_image,x_pixel-1,y_pixel,
																				map_boundary_pixel);
																		}
																		if (!(*(column_valid_u_and_v))&&
																			(y_pixel>pixel_top))
																		{
																			XPutPixel(map_image,x_pixel,y_pixel-1,
																				map_boundary_pixel);
																		}
																		*column_valid_u_and_v=1;
																	}
																	else
																	{
																		if (*(column_valid_u_and_v++)||
																			*column_valid_u_and_v)
																		{
																			XPutPixel(map_image,x_pixel,y_pixel,
																				map_boundary_pixel);
																		}
																		else
																		{
																			XPutPixel(map_image,x_pixel,y_pixel,
																				background_drawing_colour);
																		}
																		*column_valid_u_and_v=0;
																	}
																}
																else
																{
																	if (*(column_valid_u_and_v++)||
																		*column_valid_u_and_v)
																	{
																		XPutPixel(map_image,x_pixel,y_pixel,
																			map_boundary_pixel);
																	}
																	else
																	{
																		XPutPixel(map_image,x_pixel,y_pixel,
																			background_drawing_colour);
																	}
																	*column_valid_u_and_v=0;
																}
															}
															else
															{
																if (*(column_valid_u_and_v++)||
																	*column_valid_u_and_v)
																{
																	XPutPixel(map_image,x_pixel,y_pixel,
																		map_boundary_pixel);
																}
																else
																{
																	XPutPixel(map_image,x_pixel,y_pixel,
																		background_drawing_colour);
																}
																*column_valid_u_and_v=0;
															}
															x_screen += x_screen_step;
															x_pixel++;
															if (x_pixel>=next_contour_x)
															{
																contour_x++;
																contour_y++;
																next_contour_x += contour_x_spacing;
															}
														}
														y_screen += y_screen_step;
														y_pixel++;
													}
												}
												region_item=region_item->next;
											}
										}
										busy_cursor_off((Widget)NULL);
										MYFREE(row_valid_u_and_v);
									}
									else
									{
										print_message(1,
										"draw_map.  Insufficient memory for drawing map boundary");
									}
								}
								update_colour_map(map);
								if ((CONSTANT_THICKNESS==map->contour_thickness)&&
									(pixel_value=map->pixel_values))
								{
									if (SHOW_COLOUR==map->colour_option)
									{
										XPSPutImage(display,drawing->pixel_map,
											graphics_contexts.copy,map_image,0,0,0,0,drawing_width,
											drawing_height);
										draw_boundary=0;
									}
									else
									{
										if (map->print)
										{
											draw_boundary=1;
										}
										else
										{
											XPSPutImage(display,drawing->pixel_map,
												graphics_contexts.copy,map_image,0,0,0,0,drawing_width,
												drawing_height);
											draw_boundary=0;
										}
									}
									if ((SHOW_CONTOURS==map->contours_option)&&
										((contour_minimum=map->contour_minimum)<
										(contour_maximum=map->contour_maximum))&&
										(1<(number_of_contours=map->number_of_contours)))
									{
										draw_contours=1;
									}
									else
									{
										draw_contours=0;
									}
									if (draw_contours||draw_boundary)
									{
										busy_cursor_on((Widget)NULL);
										contour_step=(contour_maximum-contour_minimum)/
											(float)(number_of_contours-1);
										for (j=1;j<drawing_height;j++)
										{
											if ((background_drawing_colour==
												(pixel=XGetPixel(map_image,0,j-1)))||
												(map_boundary_pixel==pixel))
											{
												valid_i_jm1=0;
											}
											else
											{
												valid_i_jm1=1;
												f_i_jm1= *pixel_value;
											}
											if ((background_drawing_colour==
												(pixel=XGetPixel(map_image,0,j)))||
												(map_boundary_pixel==pixel))
											{
												valid_i_j=0;
											}
											else
											{
												valid_i_j=1;
												f_i_j=pixel_value[drawing_width];
											}
											pixel_value++;
											for (i=1;i<drawing_width;i++)
											{
												valid_im1_jm1=valid_i_jm1;
												f_im1_jm1=f_i_jm1;
												valid_im1_j=valid_i_j;
												f_im1_j=f_i_j;
												if ((background_drawing_colour==
													(pixel=XGetPixel(map_image,i,j-1)))||
													(map_boundary_pixel==pixel))
												{
													valid_i_jm1=0;
												}
												else
												{
													valid_i_jm1=1;
													f_i_jm1= *pixel_value;
												}
												if ((background_drawing_colour==
													(pixel=XGetPixel(map_image,i,j)))||
													(map_boundary_pixel==pixel))
												{
													valid_i_j=0;
												}
												else
												{
													valid_i_j=1;
													f_i_j=pixel_value[drawing_width];
												}
												pixel_value++;
												boundary_type=((valid_im1_jm1*2+valid_im1_j)*2+
													valid_i_jm1)*2+valid_i_j;
												if (draw_contours&&(15==boundary_type))
												{
													/* calculate contour using bilinear */
													if (f_im1_jm1<f_i_j)
													{
														min_f=f_im1_jm1;
														max_f=f_i_j;
													}
													else
													{
														min_f=f_i_j;
														max_f=f_im1_jm1;
													}
													if (f_im1_j<min_f)
													{
														min_f=f_im1_j;
													}
													else
													{
														if (f_im1_j>max_f)
														{
															max_f=f_im1_j;
														}
													}
													if (f_i_jm1<min_f)
													{
														min_f=f_i_jm1;
													}
													else
													{
														if (f_i_jm1>max_f)
														{
															max_f=f_i_jm1;
														}
													}
													if ((min_f<=contour_maximum)&&
														(contour_minimum<=max_f))
													{
														if (min_f<=contour_minimum)
														{
															start=0;
														}
														else
														{
															start=1+(int)((min_f-contour_minimum)/
																contour_step);
														}
														if (contour_maximum<=max_f)
														{
															end=number_of_contours;
														}
														else
														{
															end=(int)((max_f-contour_minimum)/contour_step);
														}
														for (k=start;k<=end;k++)
														{
															a=contour_minimum+contour_step*(float)k;
															if (((f_im1_jm1<=a)&&(a<f_i_jm1))||
																((f_im1_jm1>=a)&&(a>f_i_jm1)))
															{
																if (((f_im1_jm1<=a)&&(a<f_im1_j))||
																	((f_im1_jm1>=a)&&(a>f_im1_j)))
																{
																	if ((((f_i_jm1<=a)&&(a<f_i_j))||
																		((f_i_jm1>=a)&&(a>f_i_j)))&&
																		(((f_im1_j<=a)&&(a<f_i_j))||
																		((f_im1_j>=a)&&(a>f_i_j))))
																	{
																		b=(a-f_im1_jm1)*(f_im1_jm1+f_i_j-f_im1_j-
																			f_i_jm1)+(f_im1_j-f_im1_jm1)*
																			(f_i_jm1-f_im1_jm1);
																		if (b<0)
																		{
																			XPSDrawLineFloat(display,
																				drawing->pixel_map,
																				graphics_contexts.contour_colour,
																				(float)i-(a-f_i_jm1)/
																				(f_im1_jm1-f_i_jm1),(float)(j-1),
																				(float)(i-1),(float)j-(a-f_im1_j)/
																				(f_im1_jm1-f_im1_j));
/*																				i-(int)((a-f_i_jm1)/
																				(f_im1_jm1-f_i_jm1)+0.5),j-1,
																				i-1,j-(int)((a-f_im1_j)/
																				(f_im1_jm1-f_im1_j)+0.5));*/
																			XPSDrawLineFloat(display,
																				drawing->pixel_map,
																				graphics_contexts.contour_colour,
																				(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
																				(float)j,(float)i,(float)j-(a-f_i_j)/
																				(f_i_jm1-f_i_j));
/*																				i-(int)((a-f_i_j)/(f_im1_j-f_i_j)+0.5),
																				j,i,j-(int)((a-f_i_j)/
																				(f_i_jm1-f_i_j)+0.5));*/
																		}
																		else
																		{
																			if (b>0)
																			{
																				XPSDrawLineFloat(display,
																					drawing->pixel_map,
																					graphics_contexts.contour_colour,
																					(float)i-(a-f_i_jm1)/
																					(f_im1_jm1-f_i_jm1),(float)(j-1),
																					(float)i,(float)j-(a-f_i_j)/
																					(f_i_jm1-f_i_j));
/*																					i-(int)((a-f_i_jm1)/
																					(f_im1_jm1-f_i_jm1)+0.5),j-1,
																					i,j-(int)((a-f_i_j)/
																					(f_i_jm1-f_i_j)+0.5));*/
																				XPSDrawLineFloat(display,
																					drawing->pixel_map,
																					graphics_contexts.contour_colour,
																					(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
																					(float)j,(float)(i-1),(float)j-
																					(a-f_im1_j)/(f_im1_jm1-f_im1_j));
/*																					i-(int)((a-f_i_j)/(f_im1_j-f_i_j)+
																					0.5),j,i-1,j-
																					(int)((a-f_im1_j)/(f_im1_jm1-f_im1_j)+
																					0.5));*/
																			}
																			else
																			{
																				XPSDrawLineFloat(display,
																					drawing->pixel_map,
																					graphics_contexts.contour_colour,
																					(float)(i-1),(float)j-(a-f_im1_j)/
																					(f_im1_jm1-f_im1_j),(float)i,(float)j-
																					(a-f_i_j)/(f_i_jm1-f_i_j));
/*																					i-1,j-(int)((a-f_im1_j)/
																					(f_im1_jm1-f_im1_j)+0.5),i,j-
																					(int)((a-f_i_j)/(f_i_jm1-f_i_j)+0.5));*/
																				XPSDrawLineFloat(display,
																					drawing->pixel_map,
																					graphics_contexts.contour_colour,
																					(float)i-(a-f_i_jm1)/
																					(f_im1_jm1-f_i_jm1),(float)(j-1),
																					(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
																					(float)j);
/*																					i-(int)((a-f_i_jm1)/
																					(f_im1_jm1-f_i_jm1)+0.5),j-1,
																					i-(int)((a-f_i_j)/(f_im1_j-f_i_j)+
																					0.5),j);*/
																			}
																		}
																	}
																	else
																	{
																		XPSDrawLineFloat(display,drawing->pixel_map,
																			graphics_contexts.contour_colour,
																			(float)i-(a-f_i_jm1)/(f_im1_jm1-f_i_jm1),
																			(float)(j-1),(float)(i-1),(float)j-
																			(a-f_im1_j)/(f_im1_jm1-f_im1_j));
/*																			i-(int)((a-f_i_jm1)/(f_im1_jm1-f_i_jm1)+
																			0.5),j-1,i-1,j-
																			(int)((a-f_im1_j)/(f_im1_jm1-f_im1_j)+
																			0.5));*/
																	}
																}
																else
																{
																	if (((f_i_jm1<=a)&&(a<f_i_j))||
																		((f_i_jm1>=a)&&(a>f_i_j)))
																	{
																		XPSDrawLineFloat(display,drawing->pixel_map,
																			graphics_contexts.contour_colour,
																			(float)i-(a-f_i_jm1)/(f_im1_jm1-f_i_jm1),
																			(float)(j-1),(float)i,(float)j-
																			(a-f_i_j)/(f_i_jm1-f_i_j));
/*																			i-(int)((a-f_i_jm1)/(f_im1_jm1-f_i_jm1)+
																			0.5),j-1,i,j-
																			(int)((a-f_i_j)/(f_i_jm1-f_i_j)+0.5));*/
																	}
																	else
																	{
																		if (((f_im1_j<=a)&&(a<f_i_j))||
																			((f_im1_j>=a)&&(a>f_i_j)))
																		{
																			XPSDrawLineFloat(display,
																				drawing->pixel_map,
																				graphics_contexts.contour_colour,
																				(float)i-(a-f_i_jm1)/
																				(f_im1_jm1-f_i_jm1),(float)(j-1),
																				(float)i-(a-f_i_j)/
																				(f_im1_j-f_i_j),(float)j);
/*																				i-(int)((a-f_i_jm1)/
																				(f_im1_jm1-f_i_jm1)+0.5),j-1,
																				i-(int)((a-f_i_j)/(f_im1_j-f_i_j)+0.5),
																				j);*/
																		}
																	}
																}
															}
															else
															{
																if (((f_im1_jm1<=a)&&(a<f_im1_j))||
																	((f_im1_jm1>=a)&&(a>f_im1_j)))
																{
																	if (((f_i_jm1<=a)&&(a<f_i_j))||
																		((f_i_jm1>=a)&&(a>f_i_j)))
																	{
																		XPSDrawLineFloat(display,drawing->pixel_map,
																			graphics_contexts.contour_colour,
																			(float)(i-1),(float)j-(a-f_im1_j)/
																			(f_im1_jm1-f_im1_j),(float)i,(float)j-
																			(a-f_i_j)/(f_i_jm1-f_i_j));
/*																			i-1,j-(int)((a-f_im1_j)/
																			(f_im1_jm1-f_im1_j)+0.5),i,j-
																			(int)((a-f_i_j)/(f_i_jm1-f_i_j)+0.5));*/
																	}
																	else
																	{
																		if (((f_im1_j<=a)&&(a<f_i_j))||
																			((f_im1_j>=a)&&(a>f_i_j)))
																		{
																			XPSDrawLineFloat(display,
																				drawing->pixel_map,
																				graphics_contexts.contour_colour,
																				(float)(i-1),(float)j-(a-f_im1_j)/
																				(f_im1_jm1-f_im1_j),(float)i-
																				(a-f_i_j)/(f_im1_j-f_i_j),(float)j);
/*																				i-1,j-(int)((a-f_im1_j)/
																				(f_im1_jm1-f_im1_j)+0.5),i-
																				(int)((a-f_i_j)/(f_im1_j-f_i_j)+0.5),j);*/
																		}
																	}
																}
																else
																{
																	if ((((f_i_jm1<=a)&&(a<f_i_j))||
																		((f_i_jm1>=a)&&(a>f_i_j)))&&
																		(((f_im1_j<=a)&&(a<f_i_j))||
																		((f_im1_j>=a)&&(a>f_i_j))))
																	{
																		XPSDrawLineFloat(display,drawing->pixel_map,
																			graphics_contexts.contour_colour,
																			(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
																			(float)j,(float)i,(float)j-(a-f_i_j)/
																			(f_i_jm1-f_i_j));
/*																			i-(int)((a-f_i_j)/(f_im1_j-f_i_j)+0.5),
																			j,i,j-(int)((a-f_i_j)/
																			(f_i_jm1-f_i_j)+0.5));*/
																	}
																}
															}
														}
													}
												}
												if (draw_boundary&&(0<boundary_type)&&
													(boundary_type<15))
												{
													switch (boundary_type)
													{
														case 1: case 8:
														{
															XPSDrawLine(display,drawing->pixel_map,
																graphics_contexts.contour_colour,i-1,j,i,j-1);
														} break;
														case 2: case 4:
														{
															XPSDrawLine(display,drawing->pixel_map,
																graphics_contexts.contour_colour,i-1,j-1,i,j);
														} break;
														case 3:
														{
															XPSDrawLine(display,drawing->pixel_map,
																graphics_contexts.contour_colour,i-1,j-1,i-1,j);
														} break;
														case 5:
														{
															XPSDrawLine(display,drawing->pixel_map,
																graphics_contexts.contour_colour,i-1,j-1,i,j-1);
														} break;
														case 10:
														{
															XPSDrawLine(display,drawing->pixel_map,
																graphics_contexts.contour_colour,i-1,j,i,j);
														} break;
														case 12:
														{
															XPSDrawLine(display,drawing->pixel_map,
																graphics_contexts.contour_colour,i,j-1,i,j);
														} break;
													}
												}
											}
										}
										busy_cursor_off((Widget)NULL);
									}
								}
								else
								{
									XPSPutImage(display,drawing->pixel_map,graphics_contexts.copy,
										map_image,0,0,0,0,drawing_width,drawing_height);
								}
							}
							else
							{
								print_message(1,"draw_map.  Missing image");
							}
						}
						/* write contour values */
						if ((HIDE_COLOUR==map->colour_option)&&
							(SHOW_CONTOURS==map->contours_option))
						{
							if ((map->contour_x)&&(map->contour_y))
							{
								number_of_contour_areas=map->number_of_contour_areas;
								contour_areas_in_x=map->number_of_contour_areas_in_x;
								minimum_value=map->minimum_value;
								maximum_value=map->maximum_value;
								contour_minimum=map->contour_minimum;
								contour_maximum=map->contour_maximum;
								if (maximum_value==minimum_value)
								{
									start=0;
									end=number_of_spectrum_colours;
								}
								else
								{
									start=(int)((contour_minimum-minimum_value)/
										(maximum_value-minimum_value)*
										(float)(number_of_spectrum_colours-1)+0.5);
									end=(int)((contour_maximum-minimum_value)/
										(maximum_value-minimum_value)*
										(float)(number_of_spectrum_colours-1)+0.5);
								}
								cell_range=end-start;
								number_of_contours=map->number_of_contours;
								for (i=number_of_contours;i>0;)
								{
									i--;
									cell_number=start+(int)((float)(i*cell_range)/
										(float)(number_of_contours-1)+0.5);
									sprintf(value_string,"%.4g",(contour_maximum*(float)i+
										contour_minimum*(float)(number_of_contours-i-1))/
										(float)(number_of_contours-1));
									string_length=strlen(value_string);
									XTextExtents(font,value_string,string_length,&direction,
										&ascent,&descent,&bounds);
									x_offset=(bounds.lbearing-bounds.rbearing)/2;
									y_offset=(bounds.ascent-bounds.descent)/2;
									x_separation=(bounds.lbearing+bounds.rbearing);
									y_separation=(bounds.ascent+bounds.descent);
									contour_x=
										(map->contour_x)+(cell_number*number_of_contour_areas);
									contour_y=
										(map->contour_y)+(cell_number*number_of_contour_areas);
									for (j=0;j<number_of_contour_areas;j++)
									{
										if ((x_pixel= *contour_x)>=0)
										{
											y_pixel= *contour_y;
											/* check that its not too close to previously drawn
												values */
											draw_contour_value=1;
											if (0<j%contour_areas_in_x)
											{
												if ((pixel_left= *(contour_x-1))>=0)
												{
													temp_int=y_pixel-(*(contour_y-1));
													if (temp_int<0)
													{
														temp_int= -temp_int;
													}
													if ((x_pixel-pixel_left<=x_separation)&&
														(temp_int<=y_separation))
													{
														draw_contour_value=0;
													}
												}
												if (draw_contour_value&&(j>contour_areas_in_x)&&
													((pixel_top= *(contour_x-(contour_areas_in_x+1)))>=0))
												{
													if ((x_pixel-pixel_top<=x_separation)&&
														(y_pixel-(*(contour_y-(contour_areas_in_x+1)))<=
														y_separation))
													{
														draw_contour_value=0;
													}
												}
											}
											if (draw_contour_value&&(j>=contour_areas_in_x)&&
												((pixel_top= *(contour_x-contour_areas_in_x))>=0))
											{
												temp_int=x_pixel-pixel_top;
												if (temp_int<0)
												{
													temp_int= -temp_int;
												}
												if ((temp_int<=x_separation)&&
													(y_pixel-(*(contour_y-contour_areas_in_x))<=
													y_separation))
												{
													draw_contour_value=0;
												}
											}
											if (draw_contour_value)
											{
												XPSDrawString(display,drawing->pixel_map,
													graphics_contexts.contour_colour,x_pixel+x_offset,
													y_pixel+y_offset,value_string,string_length);
											}
										}
										contour_x++;
										contour_y++;
									}
								}
							}
						}
						/* draw the fibres */
						if ((HIDE_FIBRES!=map->fibres_option)&&(SOCK==rig_type))
						{
							/* set the colour for the fibres */
							XSetForeground(display,graphics_contexts.spectrum,
								user_settings.fibre_colour);
							/* determine the fibre spacing */
							switch (map->fibres_option)
							{
								case SHOW_FIBRES_FINE:
								{
									fibre_spacing=10;
								} break;
								case SHOW_FIBRES_MEDIUM:
								{
									fibre_spacing=20;
								} break;
								case SHOW_FIBRES_COARSE:
								{
									fibre_spacing=30;
								} break;
							}
							/* for each region */
							region_item=rig->region_list;
							for (region_number=0;region_number<number_of_regions;
								region_number++)
							{
								if (number_of_regions>1)
								{
									current_region=region_item->region;
								}
								else
								{
									current_region=get_Rig_current_region(rig);
								}
								/* draw fibres */
								pixel_left=((region_number/number_of_rows)*
									drawing_width)/number_of_columns+fibre_spacing/2;
								pixel_top=((region_number%number_of_rows)*
									drawing_height)/number_of_rows+fibre_spacing/2;
								x_screen_step=(float)fibre_spacing/stretch_x;
								y_screen_step= -(float)fibre_spacing/stretch_y;
								x_screen_left=min_x[region_number]+
									(pixel_left-start_x[region_number])/stretch_x;
								y_screen_top=min_y[region_number]-
									(pixel_top-start_y[region_number])/stretch_y;
								y_screen=y_screen_top;
								y_pixel=pixel_top;
								for (j=screen_region_height/fibre_spacing;j>0;j--)
								{
									x_screen=x_screen_left;
									x_pixel=pixel_left;
									for (i=screen_region_width/fibre_spacing;i>0;i--)
									{
										/* calculate the element coordinates and the Jacobian for
											the transformation from element coordinates to physical
											coordinates */
										switch (map->projection)
										{
											case HAMMER:
											{
												/*???avoid singularity ? */
												if ((x_screen!=0)||
													((y_screen!=1)&&(y_screen!= -1)))
												{
													a=x_screen*x_screen;
													b=y_screen*y_screen;
													c=2-a-b;
													if (c>1)
													{
														sin_mu_hat=y_screen*sqrt(c);
														if (sin_mu_hat>=1)
														{
															mu=pi;
															sin_mu_hat=1;
															cos_mu_hat=0;
														}
														else
														{
															if (sin_mu_hat<= -1)
															{
																mu=0;
																sin_mu_hat= -1;
																cos_mu_hat=0;
															}
															else
															{
																mu=pi_over_2+asin(sin_mu_hat);
																cos_mu_hat=sqrt(1-sin_mu_hat*sin_mu_hat);
															}
														}
														sin_theta_hat=sqrt((a*c)/(1-b*c));
														if (x_screen>0)
														{
															if (sin_theta_hat>=1)
															{
																theta=0;
																sin_theta_hat= -1;
																cos_theta_hat=0;
															}
															else
															{
																theta=pi-2*asin(sin_theta_hat);
																cos_theta_hat=
																	sqrt(1-sin_theta_hat*sin_theta_hat);
																sin_theta_hat= -sin_theta_hat;
															}
														}
														else
														{
															if (sin_theta_hat>=1)
															{
																theta=two_pi;
																sin_theta_hat=1;
																cos_theta_hat=0;
															}
															else
															{
																theta=pi+2*asin(sin_theta_hat);
																cos_theta_hat=
																	sqrt(1-sin_theta_hat*sin_theta_hat);
															}
														}
														a=cos_theta_hat*cos_mu_hat;
														b=1/(1+a);
														a += 2;
														b /= sqrt(b)*2;
														dxdmu=a*b*sin_mu_hat*sin_theta_hat;
														dydmu=b*(cos_theta_hat+a*cos_mu_hat);
														b /= 2;
														dxdtheta=
															-b*cos_mu_hat*(cos_mu_hat+a*cos_theta_hat);
														dydtheta= b*sin_theta_hat*sin_mu_hat*cos_mu_hat;
														valid_mu_and_theta=1;
													}
													else
													{
														valid_mu_and_theta=0;
													}
												}
												else
												{
													valid_mu_and_theta=0;
												}
											} break;
											case POLAR:
											{
												mu=sqrt(x_screen*x_screen+
													y_screen*y_screen);
												if (mu>0)
												{
													if (x_screen>0)
													{
														if (y_screen>0)
														{
															theta=asin(y_screen/mu);
														}
														else
														{
															theta=two_pi+asin(y_screen/mu);
														}
													}
													else
													{
														theta=pi-asin(y_screen/mu);
													}
													dxdtheta= -y_screen;
													dxdmu=x_screen/mu;
													dydtheta=x_screen;
													dydmu=y_screen/mu;
												}
												else
												{
													theta=0;
													dxdtheta=0;
													dxdmu=0;
													dydtheta=0;
													dydmu=0;
												}
												valid_mu_and_theta=1;
											} break;
											default:
											{
												valid_mu_and_theta=0;
											} break;
										}
										if (valid_mu_and_theta)
										{
											/* calculate the angle between the fibre direction and the
												positive theta direction */
												/*???DB.  Eventually this will form part of the
													"cardiac database" */
											/* the fibre direction has been fitted with a bilinear */
											/* determine which element the point is in */
											column=0;
											local_fibre_node_1=global_fibre_nodes+
												(NUMBER_OF_FIBRE_COLUMNS-1);
											local_fibre_node_2=global_fibre_nodes;
											local_fibre_node_3=local_fibre_node_1+
												NUMBER_OF_FIBRE_COLUMNS;
											local_fibre_node_4=local_fibre_node_2+
												NUMBER_OF_FIBRE_COLUMNS;
											k=NUMBER_OF_FIBRE_ROWS;
											xi_2= -1;
											while ((k>0)&&((xi_2<0)||(xi_2>1)||(xi_1<0)||(xi_1>1)))
											{
												l=NUMBER_OF_FIBRE_COLUMNS;
												while ((l>0)&&((xi_2<0)||(xi_2>1)||(xi_1<0)||(xi_1>1)))
												{
													/* calculate the element coordinates */
														/*???DB.  Is there a better way of doing this ? */
													mu_1=local_fibre_node_1->mu;
													mu_2=local_fibre_node_2->mu;
													mu_3=local_fibre_node_3->mu;
													mu_4=local_fibre_node_4->mu;
													if (((mu_1<mu)||(mu_2<mu)||(mu_3<mu)||(mu_4<mu))&&
														((mu_1>mu)||(mu_2>mu)||(mu_3>mu)||(mu_4>mu)))
													{
														theta_1=local_fibre_node_1->theta;
														theta_2=local_fibre_node_2->theta;
														theta_3=local_fibre_node_3->theta;
														theta_4=local_fibre_node_4->theta;
														/* make sure that theta is increasing in xi1 */
														if (theta_1>theta_2)
														{
															if (theta>theta_1)
															{
																theta_2 += two_pi;
															}
															else
															{
																theta_1 -= two_pi;
															}
														}
														if (theta_3>theta_4)
														{
															if (theta>theta_3)
															{
																theta_4 += two_pi;
															}
															else
															{
																theta_3 -= two_pi;
															}
														}
														if (theta_1+pi<theta_3)
														{
															if (theta>theta_3)
															{
																theta_1 += two_pi;
																theta_2 += two_pi;
															}
															else
															{
																theta_3 -= two_pi;
																theta_4 -= two_pi;
															}
														}
														else
														{
															if (theta_1-pi>theta_3)
															{
																if (theta>theta_1)
																{
																	theta_3 += two_pi;
																	theta_4 += two_pi;
																}
																else
																{
																	theta_1 -= two_pi;
																	theta_2 -= two_pi;
																}
															}
														}
														if (((theta_1<theta)||(theta_2<theta)||
															(theta_3<theta)||(theta_4<theta))&&
															((theta_1>theta)||(theta_2>theta)||
															(theta_3>theta)||(theta_4>theta)))
														{
															mu_4 += mu_1-mu_2-mu_3;
															mu_3 -= mu_1;
															mu_2 -= mu_1;
															mu_1 -= mu;
															theta_4 += theta_1-theta_2-theta_3;
															theta_3 -= theta_1;
															theta_2 -= theta_1;
															theta_1 -= theta;
															xi_1=0.5;
															xi_2=0.5;
															fibre_iteration=0;
															do
															{
																a=mu_2+mu_4*xi_2;
																b=mu_3+mu_4*xi_1;
																error_mu=mu_1+mu_2*xi_1+b*xi_2;
																c=theta_2+theta_4*xi_2;
																d=theta_3+theta_4*xi_1;
																error_theta=theta_1+theta_2*xi_1+d*xi_2;
																if (0!=(det=a*d-b*c))
																{
																	xi_1 -= (d*error_mu-b*error_theta)/det;
																	xi_2 -= (a*error_theta-c*error_mu)/det;
																}
																fibre_iteration++;
															} while ((0!=det)&&(error_theta*error_theta+
																error_mu*error_mu>FIBRE_TOLERANCE)&&
																(fibre_iteration<FIBRE_MAXIMUM_ITERATIONS));
															if (error_theta*error_theta+error_mu*error_mu>
																FIBRE_TOLERANCE)
															{
																xi_2= -1;
															}
														}
													}
													if ((xi_2<0)||(xi_2>1)||(xi_1<0)||(xi_1>1))
													{
														local_fibre_node_1=local_fibre_node_2;
														local_fibre_node_2++;
														local_fibre_node_3=local_fibre_node_4;
														local_fibre_node_4++;
														l--;
													}
												}
												if ((xi_2<0)||(xi_2>1)||(xi_1<0)||(xi_1>1))
												{
													local_fibre_node_1=local_fibre_node_3;
													local_fibre_node_3 += NUMBER_OF_FIBRE_COLUMNS;
													k--;
												}
											}
											if ((0<=xi_1)&&(xi_1<=1)&&(0<=xi_2)&&(xi_2<=1))
											{
/*???debug */
/*printf("k=%d, l=%d\n",k,l);
printf("xi_1=%g, xi_2=%g\n",xi_1,xi_2);
printf("mu_1=%g, mu_2=%g, mu_3=%g, mu_4=%g\n",local_fibre_node_1->mu,
	local_fibre_node_2->mu,local_fibre_node_3->mu,local_fibre_node_4->mu);
printf("mu_1=%g, mu_2=%g, mu_3=%g, mu_4=%g\n",mu_1,mu_2,mu_3,mu_4);
printf("theta_1=%g, theta_2=%g, theta_3=%g, theta_4=%g\n",
	local_fibre_node_1->theta,local_fibre_node_2->theta,local_fibre_node_3->theta,
	local_fibre_node_4->theta);
printf("theta_1=%g, theta_2=%g, theta_3=%g, theta_4=%g\n",theta_1,theta_2,
	theta_3,theta_4);
printf("x_pixel=%d, y_pixel=%d\n",x_pixel,y_pixel);
printf("mu=%g, theta=%g\n",mu,theta);
printf("dxdmu=%g, dxdtheta=%g, dydmu=%g, dydtheta=%g\n",dxdmu,dxdtheta,dydmu,
	dydtheta);*/
												dmudxi1=mu_2+mu_4*xi_2;
												dmudxi2=mu_3+mu_4*xi_1;
												dthetadxi1=theta_2+theta_4*xi_2;
												dthetadxi2=theta_3+theta_4*xi_1;
/*???debug */
/*printf("dmudxi1=%g, dmudxi2=%g, dthetadxi1=%g, dthetadxi2=%g\n",dmudxi1,dmudxi2,
	dthetadxi1,dthetadxi2);*/
												/* calculate the fibre angle */
												fibre_angle_1=local_fibre_node_1->fibre_angle;
												fibre_angle_2=local_fibre_node_2->fibre_angle;
												fibre_angle_3=local_fibre_node_3->fibre_angle;
												fibre_angle_4=local_fibre_node_4->fibre_angle;
												fibre_angle=fibre_angle_1+
													(fibre_angle_2-fibre_angle_1)*xi_1+
													((fibre_angle_3-fibre_angle_1)+
													(fibre_angle_4+fibre_angle_1-fibre_angle_2-
													fibre_angle_3)*xi_1)*xi_2;
												/* calculate the fibre vector in element coordinates */
/*												a=cos(fibre_angle);
												b=sin(fibre_angle);*/
/*???debug */
/*printf("fibre angle=%g, a=%g, b=%g\n",fibre_angle,a,b);*/
												/* transform to prolate */
/*												c=dmudxi1*a+dmudxi2*b;
												a=dthetadxi1*a+dthetadxi2*b;*/
												a=cos(fibre_angle);
												c=sin(fibre_angle);
												/* perform projection and screen scaling */
												fibre_x=stretch_x*(dxdmu*c+dxdtheta*a);
												fibre_y= -stretch_y*(dydmu*c+dydtheta*a);
												if (0<(fibre_length=fibre_x*fibre_x+fibre_y*fibre_y))
												{
													/* draw the fibre */
													fibre_length=
														(float)(fibre_spacing)/(2*sqrt(fibre_length));
													fibre_x *= fibre_length;
													fibre_y *= fibre_length;
													XPSDrawLine(display,drawing->pixel_map,
														graphics_contexts.spectrum,
														x_pixel-(short)fibre_x,y_pixel-(short)fibre_y,
														x_pixel+(short)fibre_x,y_pixel+(short)fibre_y);
												}
/*???debug */
/*printf("fibre_x=%g, fibre_y=%g\n\n",fibre_x,fibre_y);*/
											}
										}
										x_screen += x_screen_step;
										x_pixel += fibre_spacing;
									}
									y_screen += y_screen_step;
									y_pixel += fibre_spacing;
								}
								region_item=region_item->next;
							}
						}
						/* draw the landmarks */
						if ((SHOW_LANDMARKS==map->landmarks_option)&&(SOCK==rig_type))
						{
							/* set the colour for the landmarks */
							XSetForeground(display,graphics_contexts.spectrum,
								user_settings.landmark_colour);
							landmark_point=landmark_points;
							for (i=NUMBER_OF_LANDMARK_POINTS;i>0;i--)
							{
								cartesian_to_prolate_spheroidal(landmark_point[0],
									landmark_point[1],landmark_point[2],LANDMARK_FOCUS,&lambda,
									&mu,&theta);
								switch (map->projection)
								{
									case HAMMER:
									{
										Hammer_projection(mu,theta,&x_screen,&y_screen);
									} break;
									case POLAR:
									{
										polar_projection(mu,theta,&x_screen,&y_screen);
									} break;
								}
								for (region_number=0;region_number<number_of_regions;
									region_number++)
								{
									x_pixel=start_x[region_number]+
										(int)((x_screen-min_x[region_number])*stretch_x);
									y_pixel=start_y[region_number]-
										(int)((y_screen-min_y[region_number])*stretch_y);
									/* draw asterisk */
									XPSDrawLine(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel-2,y_pixel,x_pixel+2,y_pixel);
									XPSDrawLine(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel,y_pixel-2,x_pixel,y_pixel+2);
									XPSDrawLine(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel-2,y_pixel-2,x_pixel+2,y_pixel+2);
									XPSDrawLine(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel+2,y_pixel+2,x_pixel-2,y_pixel-2);
								}
								landmark_point += 3;
							}
							/*???DB. temp for finding bifurcations */
							/* set the colour for the bifurcations */
							XSetForeground(display,graphics_contexts.spectrum,
								user_settings.potential_time_colour);
							landmark_point=bifurcation_points;
							for (i=NUMBER_OF_BIFURCATION_POINTS;i>0;i--)
							{
								cartesian_to_prolate_spheroidal(landmark_point[0],
									landmark_point[1],landmark_point[2],LANDMARK_FOCUS,&lambda,
									&mu,&theta);
								switch (map->projection)
								{
									case HAMMER:
									{
										Hammer_projection(mu,theta,&x_screen,&y_screen);
									} break;
									case POLAR:
									{
										polar_projection(mu,theta,&x_screen,&y_screen);
									} break;
								}
								for (region_number=0;region_number<number_of_regions;
									region_number++)
								{
									x_pixel=start_x[region_number]+
										(int)((x_screen-min_x[region_number])*stretch_x);
									y_pixel=start_y[region_number]-
										(int)((y_screen-min_y[region_number])*stretch_y);
									/* draw asterisk */
									XPSDrawLine(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel-2,y_pixel,x_pixel+2,y_pixel);
									XPSDrawLine(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel,y_pixel-2,x_pixel,y_pixel+2);
									XPSDrawLine(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel-2,y_pixel-2,x_pixel+2,y_pixel+2);
									XPSDrawLine(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel+2,y_pixel+2,x_pixel-2,y_pixel-2);
									/* write number */
									sprintf(value_string,"%d",NUMBER_OF_BIFURCATION_POINTS-i+1);
									XPSDrawString(display,drawing->pixel_map,
										graphics_contexts.spectrum,
										x_pixel,y_pixel,value_string,strlen(value_string));
								}
								landmark_point += 3;
							}
						}
						/* draw electrodes last */
						if (HIDE_ELECTRODES!=map->electrodes_option)
						{
							/* for each electrode draw a 'plus' at its position and its name
								above */
#if !defined (NO_ALIGNMENT)
							SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
							SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
#endif
							electrode=map->electrodes;
							x_item=x;
							y_item=y;
							screen_x=map->electrode_x;
							screen_y=map->electrode_y;
							electrode_drawn=map->electrode_drawn;
							min_f=map->minimum_value;
							max_f=map->maximum_value;
							if ((range_f=max_f-min_f)<=0)
							{
								range_f=1;
							}
							for (;number_of_electrodes>0;number_of_electrodes--)
							{
								if (current_region)
								{
									region_number=0;
								}
								else
								{
									region_number=(*electrode)->description->region->number;
								}
								/* calculate screen position */
								*screen_x=start_x[region_number]+
									(int)(((*x_item)-min_x[region_number])*stretch_x);
								*screen_y=start_y[region_number]-
									(int)(((*y_item)-min_y[region_number])*stretch_y);
								switch (map->electrodes_option)
								{
									case SHOW_ELECTRODE_NAMES:
									case SHOW_CHANNEL_NUMBERS:
									{
										*electrode_drawn=1;
										if (SHOW_ELECTRODE_NAMES==map->electrodes_option)
										{
											name=(*electrode)->description->name;
										}
										else
										{
											sprintf(value_string,"%d",(*electrode)->channel->number);
											name=value_string;
										}
										if ((*electrode)->highlight)
										{
											graphics_context= &(graphics_contexts.highlighted_colour);
										}
										else
										{
											graphics_context= 
											&(graphics_contexts.unhighlighted_colour);
										}
									} break;
									case SHOW_ELECTRODE_VALUES:
									{
										/* calculate value */
										switch (map_type)
										{
											case ACTIVATION:
											{
												event=(*electrode)->signal->first_event;
												while (event&&(event->number<event_number))
												{
													event=event->next;
												}
												if (event&&(event->number==event_number)&&
													((event->status==ACCEPTED)||
													(undecided_accepted&&(event->status==UNDECIDED))))
												{
													*electrode_drawn=1;
													f_value=(float)((event->time)-datum)*1000/
														((*electrode)->signal->buffer->frequency);
												}
												else
												{
													*electrode_drawn=0;
												}
											} break;
											case POTENTIAL:
											{
												if ((signal=(*electrode)->signal)&&
													(0<=potential_time)&&
													(potential_time<signal->buffer->number_of_samples)&&
													((signal->status==ACCEPTED)||(undecided_accepted&&
													(signal->status==UNDECIDED))))
												{
													*electrode_drawn=1;
													switch (signal->buffer->value_type)
													{
														case SHORT_INT_VALUE:
														{
															f_value=((float)((signal->buffer->signals.
																short_int_values)[potential_time*
																(signal->buffer->number_of_signals)+
																(signal->index)])-
																((*electrode)->channel->offset))*
																((*electrode)->channel->gain);
														} break;
														case FLOAT_VALUE:
														{
															f_value=(((signal->buffer->signals.float_values)
																[potential_time*
																(signal->buffer->number_of_signals)+
																(signal->index)])-
																((*electrode)->channel->offset))*
																((*electrode)->channel->gain);
														} break;
													}
												}
												else
												{
													*electrode_drawn=0;
												}
											} break;
											default:
											{
												*electrode_drawn=0;
											}
										}
										if (*electrode_drawn)
										{
											sprintf(value_string,"%.4g",f_value);
											name=value_string;
											if ((*electrode)->highlight)
											{
												graphics_context=
													&(graphics_contexts.highlighted_colour);
											}
											else
											{
												if ((map->colour_option==HIDE_COLOUR)&&
													(map->contours_option==SHOW_CONTOURS))
												{
													graphics_context=
														&(graphics_contexts.unhighlighted_colour);
												}
												else
												{
													if (f_value<=min_f)
													{
														XSetForeground(display,graphics_contexts.spectrum,
															spectrum_pixels[0]);
													}
													else
													{
														if (f_value>=max_f)
														{
															XSetForeground(display,graphics_contexts.spectrum,
																spectrum_pixels[number_of_spectrum_colours-1]);
														}
														else
														{
															XSetForeground(display,graphics_contexts.spectrum,
																spectrum_pixels[(int)((f_value-min_f)*
																(float)(number_of_spectrum_colours-1)/
																range_f)]);
														}
													}
													graphics_context= &(graphics_contexts.spectrum);
												}
											}
										}
									} break;
									default:
									{
										*electrode_drawn=0;
										name=(char *)NULL;
									} break;
								}
								if (*electrode_drawn)
								{
									/* draw plus */
									XPSDrawLine(display,drawing->pixel_map,*graphics_context,
										*screen_x-2,*screen_y,*screen_x+2,*screen_y);
									XPSDrawLine(display,drawing->pixel_map,*graphics_context,
										*screen_x,*screen_y-2,*screen_x,*screen_y+2);
									if (name)
									{
										/* write name */
										name_length=strlen(name);
#if defined (NO_ALIGNMENT)
										XTextExtents(font,name,name_length,&direction,&ascent,
											&descent,&bounds);
										x_string= (*screen_x)+(bounds.lbearing-bounds.rbearing+1)/2;
										y_string= (*screen_y)-descent-1;
										/* make sure that the string doesn't extend outside the
											window */
										if (x_string-bounds.lbearing<0)
										{
											x_string=bounds.lbearing;
										}
										else
										{
											if (x_string+bounds.rbearing>drawing_width)
											{
												x_string=drawing_width-bounds.rbearing;
											}
										}
										if (y_string-ascent<0)
										{
											y_string=ascent;
										}
										else
										{
											if (y_string+descent>drawing_height)
											{
												y_string=drawing_height-descent;
											}
										}
#endif
										if ((*electrode)->highlight)
										{
											XPSDrawString(display,drawing->pixel_map,
												graphics_contexts.highlighted_colour,
#if defined (NO_ALIGNMENT)
												x_string,y_string,name,name_length);
#else
												(*screen_x),(*screen_y)-2,name,name_length);
#endif
										}
										else
										{
											XPSDrawString(display,drawing->pixel_map,
												graphics_contexts.device_name_colour,
#if defined (NO_ALIGNMENT)
												x_string,y_string,name,name_length);
#else
												(*screen_x),(*screen_y)-2,name,name_length);
#endif
										}
									}
								}
								electrode++;
								x_item++;
								y_item++;
								screen_x++;
								screen_y++;
								electrode_drawn++;
							}
						}
						MYFREE(x);
						MYFREE(y);
						MYFREE(first);
						MYFREE(max_x);
						MYFREE(min_x);
						MYFREE(max_y);
						MYFREE(min_y);
						MYFREE(start_x);
						MYFREE(start_y);
					}
					else
					{
						MYFREE(x);
						MYFREE(y);
						MYFREE(map->electrodes);
						MYFREE(map->electrode_x);
						MYFREE(map->electrode_y);
						MYFREE(first);
						MYFREE(max_x);
						MYFREE(min_x);
						MYFREE(max_y);
						MYFREE(min_y);
						MYFREE(start_x);
						MYFREE(start_y);
						print_message(1,
							"draw_map.  Could not allocate x and/or y and/or electrodes");
						return_code=0;
					}
				}
			}
		}
	}
	else
	{
		print_message(1,"draw_map.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_map */

int draw_colour_or_auxiliary_area(struct Map *map,struct Drawing *drawing)
/*******************************************************************************
LAST MODIFIED : 14 August 1994

DESCRIPTION :
This function draws the colour bar or the auxiliary inputs in the <drawing>.
It should not be called until draw_map has been called.
==============================================================================*/
{
	int ascent,colour_bar_bottom,colour_bar_left,colour_bar_right,colour_bar_top,
		descent,direction,first,i,name_end,number_of_auxiliary,number_of_devices,
		number_of_regions,region_number,return_code,*screen_x,*screen_y,
		string_length,text_y,x,xmarker,x_max,x_min,x_range,y_max,y_min,yheight,
		ymarker;
	float contour_maximum,contour_minimum,maximum_value,minimum_value,
		spectrum_left,spectrum_right,text_x;
	struct Device_description *description;
	struct Device **auxiliary,**device;
	struct Region *current_region;
	struct Region_list_item *region_item;
	struct Rig *rig;
	XCharStruct bounds;
	char value_string[11];
	struct Interpolation_function *function;

	ENTER(draw_colour_or_auxiliary_area);
	return_code=1;
	/* check arguments */
	if (map&&drawing)
	{
		/* clear the colour or auxiliary drawing area */
		XPSFillRectangle(display,drawing->pixel_map,
			graphics_contexts.background_drawing_colour,
			0,0,drawing->width,drawing->height);
		if ((map->colour_option==SHOW_COLOUR)||
			(map->contours_option==SHOW_CONTOURS))
		{
			/* clear the auxiliary positions */
			if (map->number_of_auxiliary)
			{
				map->number_of_auxiliary=0;
				MYFREE(map->auxiliary);
				MYFREE(map->auxiliary_x);
				MYFREE(map->auxiliary_y);
			}
			/* draw the colour bar */
				/*???Use XImage ? */
			if (map->rig_pointer)
			{
				if (rig= *(map->rig_pointer))
				{
					if (current_region=get_Rig_current_region(rig))
					{
						number_of_regions=1;
					}
					else
					{
						number_of_regions=rig->number_of_regions;
					}
					/* calculate the interpolation function(s) and determine the range of
						function values */
/*???done in draw_map */
/*					min_f=1;
					max_f=0;
					region_item=rig->region_list;
					for (region_number=0;region_number<number_of_regions;region_number++)
					{
						if (number_of_regions>1)
						{
							current_region=region_item->region;
						}
						else
						{
							current_region=rig->current_region;
						}*/
						/* interpolate data */
/*						if (function=current_region->interpolation_function)
						{
							if (min_f>max_f)
							{
								min_f=function->f_min;
								max_f=function->f_max;
							}
							else
							{
								if (function->f_min<min_f)
								{
									min_f=function->f_min;
								}
								if (max_f<function->f_max)
								{
									max_f=function->f_max;
								}
							}
						}
						region_item=region_item->next;
					}
					map->minimum_value=min_f;
					map->maximum_value=max_f;
					map->contour_minimum=min_f;
					map->contour_maximum=max_f;*/
					minimum_value=map->minimum_value;
					maximum_value=map->maximum_value;
					contour_minimum=map->contour_minimum;
					contour_maximum=map->contour_maximum;
					colour_bar_left=widget_spacing;
					colour_bar_right=drawing->width-widget_spacing;
					spectrum_left=(float)colour_bar_left+
						(contour_minimum-minimum_value)*
						(float)(colour_bar_right-colour_bar_left)/
						(maximum_value-minimum_value);
					spectrum_right=(float)colour_bar_left+
						(contour_maximum-minimum_value)*
						(float)(colour_bar_right-colour_bar_left)/
						(maximum_value-minimum_value);
					/* write the minimum value */
					sprintf(value_string,"%.4g",contour_minimum);
					string_length=strlen(value_string);
#if defined (NO_ALIGNMENT)
					XTextExtents(font,value_string,string_length,&direction,&ascent,
						&descent,&bounds);
					text_x=spectrum_left-(float)bounds.rbearing;
					if (text_x+(float)bounds.lbearing<(float)colour_bar_left)
					{
						text_x=(float)(colour_bar_left-bounds.lbearing);
					}
					text_y=widget_spacing+ascent;
#else
					SET_HORIZONTAL_ALIGNMENT(RIGHT_ALIGNMENT);
					SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
#endif
					XPSDrawString(display,drawing->pixel_map,
						graphics_contexts.colour_bar_text_colour,
#if defined (NO_ALIGNMENT)
						(int)(text_x+0.5),text_y,value_string,string_length);
#else
						(int)(spectrum_left+0.5),widget_spacing,value_string,string_length);
#endif
					/* write the maximum value */
					sprintf(value_string,"%.4g",contour_maximum);
					string_length=strlen(value_string);
#if defined (NO_ALIGNMENT)
					XTextExtents(font,value_string,string_length,&direction,&ascent,
						&descent,&bounds);
					text_x=spectrum_right-(float)bounds.lbearing;
					if (text_x+(float)bounds.rbearing>(float)colour_bar_right)
					{
						text_x=(float)(colour_bar_right-bounds.rbearing);
					}
					text_y=widget_spacing+ascent;
#else
					SET_HORIZONTAL_ALIGNMENT(LEFT_ALIGNMENT);
#endif
					XPSDrawString(display,drawing->pixel_map,
						graphics_contexts.colour_bar_text_colour,
#if defined (NO_ALIGNMENT)
						(int)(text_x+0.5),text_y,value_string,string_length);
#else
						(int)(spectrum_right+0.5),widget_spacing,value_string,
						string_length);
#endif
					colour_bar_top=text_y+descent+2*widget_spacing;
					colour_bar_bottom=drawing->height-widget_spacing;
					/* draw the colour bar */
					if ((colour_bar_left<colour_bar_right)&&
						(colour_bar_top<colour_bar_bottom))
					{
						x_range=colour_bar_right-colour_bar_left;
						for (x=colour_bar_left;x<=colour_bar_right;x++)
						{
							XSetForeground(display,graphics_contexts.spectrum,
								spectrum_pixels[(int)((float)((x-colour_bar_left)*
								(number_of_spectrum_colours-1))/(float)x_range+0.5)]);
							XPSDrawLine(display,drawing->pixel_map,graphics_contexts.spectrum,
								x,colour_bar_top,x,colour_bar_bottom);
						}
					}
					if ((SHOW_CONTOURS==map->contours_option)&&
						(2<map->number_of_contours)&&(contour_minimum<contour_maximum))
					{
						/* draw the contour markers */
#if !defined (NO_ALIGNMENT)
						SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
#endif
						XPSDrawLineFloat(display,drawing->pixel_map,
							graphics_contexts.contour_colour,spectrum_left,
							(float)(colour_bar_top),spectrum_left,(float)colour_bar_bottom);
						for (i=(map->number_of_contours)-2;i>0;i--)
						{
							text_x=(spectrum_right*(float)i+spectrum_left*
								(float)(map->number_of_contours-i-1))/
								(float)(map->number_of_contours-1);
							XPSDrawLineFloat(display,drawing->pixel_map,
								graphics_contexts.contour_colour,text_x,
								(float)colour_bar_top,text_x,(float)colour_bar_bottom);
							/* write the maximum value */
							sprintf(value_string,"%.4g",(contour_maximum*(float)i+
								contour_minimum*(float)(map->number_of_contours-i-1))/
								(float)(map->number_of_contours-1));
							string_length=strlen(value_string);
#if defined (NO_ALIGNMENT)
							XTextExtents(font,value_string,string_length,&direction,&ascent,
								&descent,&bounds);
							text_x += (float)(bounds.lbearing-bounds.rbearing)/2;
							text_y=widget_spacing+ascent;
#endif
							XPSDrawString(display,drawing->pixel_map,
								graphics_contexts.contour_colour,
#if defined (NO_ALIGNMENT)
								(int)(text_x+0.5),text_y,value_string,string_length);
#else
								(int)(text_x+0.5),widget_spacing,value_string,
								string_length);
#endif
						}
						XPSDrawLineFloat(display,drawing->pixel_map,
							graphics_contexts.contour_colour,spectrum_right,
							(float)(colour_bar_top),spectrum_right,(float)colour_bar_bottom);
					}
					/* draw the spectrum left and right markers */
					XPSDrawLineFloat(display,drawing->pixel_map,
						graphics_contexts.colour_bar_marker_colour,spectrum_left,
						(float)(colour_bar_top-widget_spacing),spectrum_left,
						(float)colour_bar_top);
					XPSDrawLineFloat(display,drawing->pixel_map,
						graphics_contexts.colour_bar_marker_colour,spectrum_right,
						(float)(colour_bar_top-widget_spacing),spectrum_right,
						(float)colour_bar_top);
					/* save values */
					map->colour_bar_left=colour_bar_left;
					map->colour_bar_right=colour_bar_right;
					map->colour_bar_bottom=colour_bar_bottom;
					map->colour_bar_top=colour_bar_top;
				}
			}
			else
			{
				print_message(1,
					"draw_colour_or_auxiliary_area.  NULL map->rig_pointer");
				return_code=0;
			}
			/*??? more */
		}
		else
		{
			/* draw auxiliary devices */
			if (map->rig_pointer)
			{
				if (rig= *(map->rig_pointer))
				{
					current_region=get_Rig_current_region(rig);
					/* count the auxiliary_devices */
					number_of_auxiliary=0;
					number_of_devices=rig->number_of_devices;
					device=rig->devices;
					while (number_of_devices>0)
					{
						if (((description=(*device)->description)->type==AUXILIARY)&&
							(!current_region||(current_region==description->region)))
						{
							number_of_auxiliary++;
						}
						device++;
						number_of_devices--;
					}
					map->number_of_auxiliary=number_of_auxiliary;
					if (number_of_auxiliary>0)
					{
						if (MYMALLOC(map->auxiliary,struct Device *,number_of_auxiliary)&&
							MYMALLOC(map->auxiliary_x,int,number_of_auxiliary)&&
							MYMALLOC(map->auxiliary_y,int,number_of_auxiliary))
						{
							/* calculate the positions for the auxiliary device markers */
							device=rig->devices;
							screen_x=map->auxiliary_x;
							screen_y=map->auxiliary_y;
							auxiliary=map->auxiliary;
							/* position of the auxiliary device marker */
							xmarker=4;
							ymarker=4;
							yheight=5;
							first=1;
							number_of_devices=rig->number_of_devices;
							while (number_of_devices>0)
							{
								if (((description=(*device)->description)->type==AUXILIARY)&&
									(!current_region||(current_region==description->region)))
								{
									/* add device */
									*auxiliary= *device;
									/* calculate the position of the end of the name */
									if (description->name)
									{
										string_length=strlen(description->name);
										XTextExtents(font,description->name,string_length,
											&direction,&ascent,&descent,&bounds);
										name_end=bounds.rbearing-bounds.lbearing+12;
									}
									else
									{
										name_end=5;
									}
									if ((xmarker+name_end>=drawing->width)&&(!first))
									{
										xmarker=4;
										ymarker += yheight+10;
										yheight=5;
										first=1;
									}
									if (description->name)
									{
										if (ascent-descent+1>yheight)
										{
											yheight=ascent-descent+1;
										}
									}
									/* add the position */
									*screen_x=xmarker;
									*screen_y=ymarker;
									/* draw the marker */
									if (ymarker-2<drawing->height)
									{
										if ((*auxiliary)->highlight)
										{
											XPSFillRectangle(display,drawing->pixel_map,
												graphics_contexts.highlighted_colour,
												xmarker-2,ymarker-2,5,5);
											if (description->name)
											{
												XPSDrawString(display,drawing->pixel_map,
													graphics_contexts.highlighted_colour,
													xmarker+6-bounds.lbearing,
													ymarker+ascent-4,
													description->name,string_length);
											}
										}
										else
										{
											XPSFillRectangle(display,drawing->pixel_map,
												graphics_contexts.unhighlighted_colour,
												xmarker-2,ymarker-2,5,5);
											if (description->name)
											{
												XPSDrawString(display,drawing->pixel_map,
													graphics_contexts.device_name_colour,
													xmarker+6-bounds.lbearing,
													ymarker+ascent-4,
													description->name,string_length);
											}
										}
									}
									/* calculate the position of the next marker */
									xmarker += name_end+10;
									if (xmarker-2>drawing->width)
									{
										xmarker=4;
										ymarker += yheight+10;
										yheight=5;
										first=1;
									}
									else
									{
										first=0;
									}
									auxiliary++;
									screen_x++;
									screen_y++;
								}
								device++;
								number_of_devices--;
							}
						}
						else
						{
							print_message(1,
		"draw_colour_or_auxiliary_area.  Could not allocate auxiliary information");
							return_code=0;
						}
					}
				}
			}
			else
			{
				print_message(1,
					"draw_colour_or_auxiliary_area.  NULL map->rig_pointer");
				return_code=0;
			}
		}
	}
	else
	{
		print_message(1,"draw_colour_or_auxiliary_area.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_colour_or_auxiliary_area */
