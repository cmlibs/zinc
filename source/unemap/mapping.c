/*******************************************************************************
FILE : mapping.c

LAST MODIFIED : 18 July 2002

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include <string.h>
#include <math.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#endif /* defined (MOTIF) */
#include "command/parser.h"
#include "finite_element/finite_element.h"
	/*???DB.  For fuzzy_string_compare */
#include "general/debug.h"
#include "general/geometry.h"
#include "general/postscript.h"
#include "general/mystring.h"
#if defined (UNEMAP_USE_3D)
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_component_operations.h"
#include "computed_field/computed_field_update.h"
#include "computed_field/computed_field_vector_operations.h"
#include "graphics/graphics_window.h"
#include "graphics/graphical_element.h"
#include "graphics/element_group_settings.h"
#endif /* defined (UNEMAP_USE_3D) */
#include "graphics/spectrum.h"
#include "time/time.h"
#include "unemap/drawing_2d.h"
#include "unemap/delaunay.h"
#include "unemap/interpolate.h"
#include "unemap/mapping.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "unemap/unemap_package.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/

/* for map fit field */
char *fit_comp_name[1]=
{
	"fitted_value"
};
int fit_components_number_of_derivatives[1]={3},
	fit_components_number_of_versions[1]={1};
enum FE_nodal_value_type fit_bicubic_component[4]=
	{FE_NODAL_VALUE,FE_NODAL_D_DS1,FE_NODAL_D_DS2,FE_NODAL_D2_DS1DS2};

/*
Module constants
----------------
*/
#define MAX_SPECTRUM_COLOURS 256
/*
Module types
------------
*/
typedef struct Map Map_settings;
typedef struct Map_drawing_information Map_drawing_information_settings;

#if defined (UNEMAP_USE_3D)
struct Define_fit_field_at_torso_element_nodes_data
{
	struct GROUP(FE_element) *quad_element_group;
	struct FE_field *fit_field;
	struct MANAGER(FE_node) *node_manager;
}; /* define_fit_field_at_torso_element_nodes_data */

struct Map_value_torso_data
/*******************************************************************************
LAST MODIFIED : 16 October 2000

DESCRIPTION :
used by set_map_fit_field_at_torso_node
==============================================================================*/
{
	int first_torso_element_number;
	struct Map_3d_package *map_3d_package;
	struct FE_element *torso_element;
	struct LIST(FE_node) *processed_nodes;
};

struct Element_torso_data
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
used by add_torso_elements_if_nodes_in_z_range
==============================================================================*/
{
	struct LIST(FE_node) *torso_nodes_in_z_range;
	struct GROUP(FE_element) *mapped_torso_element_group;
};


struct Z_value_data
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :

==============================================================================*/
{
	FE_value map_min_z,map_max_z;
	struct FE_field *torso_position_field;
};

struct merge_fit_field_template_element_data
/*******************************************************************************
LAST MODIFIED : 24 October 2000

DESCRIPTION :
==============================================================================*/
{
	int count;
	struct FE_element *template_element;
	struct MANAGER(FE_element) *element_manager;
};
#endif /* defined (UNEMAP_USE_3D)*/

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

static float landmark_points[3*NUMBER_OF_LANDMARK_POINTS]=
	{
		-0.340E+02,-0.262E+01,-0.146E+02,
		-0.339E+02,-0.453E+01,-0.160E+02,
		-0.331E+02,-0.745E+01,-0.165E+02,
		-0.317E+02,-0.720E+01,-0.191E+02,
		-0.297E+02,-0.753E+01,-0.228E+02,
		-0.279E+02,-0.773E+01,-0.265E+02,
		-0.256E+02,-0.801E+01,-0.297E+02,
		-0.230E+02,-0.801E+01,-0.324E+02,
		-0.209E+02,-0.787E+01,-0.339E+02,
		-0.182E+02,-0.736E+01,-0.363E+02,
		-0.147E+02,-0.603E+01,-0.378E+02,
		-0.119E+02,-0.484E+01,-0.391E+02,
		-0.960E+01,-0.353E+01,-0.399E+02,
		-0.650E+01,-0.130E+01,-0.404E+02,
		-0.370E+01, 0.399E+00,-0.409E+02,
		-0.160E+01, 0.295E+01,-0.405E+02,
		0.600E+00, 0.519E+01,-0.401E+02,
		0.260E+01, 0.740E+01,-0.396E+02,
		0.470E+01, 0.927E+01,-0.389E+02,
		0.690E+01, 0.108E+02,-0.380E+02,
		0.880E+01, 0.119E+02,-0.372E+02,
		0.110E+02, 0.131E+02,-0.362E+02,
		0.135E+02, 0.136E+02,-0.353E+02,
		0.156E+02, 0.139E+02,-0.347E+02,
		0.176E+02, 0.143E+02,-0.336E+02,
		0.198E+02, 0.146E+02,-0.324E+02,
		0.220E+02, 0.148E+02,-0.312E+02,
		0.244E+02, 0.148E+02,-0.301E+02,
		0.267E+02, 0.144E+02,-0.287E+02,
		0.290E+02, 0.140E+02,-0.275E+02,
		0.314E+02, 0.128E+02,-0.260E+02,
		0.339E+02, 0.115E+02,-0.250E+02,
		0.361E+02, 0.102E+02,-0.239E+02,
		0.384E+02, 0.907E+01,-0.221E+02,
		0.404E+02, 0.796E+01,-0.206E+02,
		0.428E+02, 0.599E+01,-0.190E+02,
		0.452E+02, 0.367E+01,-0.181E+02,
		0.473E+02, 0.217E+01,-0.149E+02,
		0.488E+02, 0.893E+00,-0.131E+02,
		0.501E+02,-0.591E+00,-0.116E+02,
		0.460E+02, 0.184E+01,-0.167E+02,
		0.464E+02,-0.472E-01,-0.168E+02,
		0.463E+02,-0.158E+01,-0.167E+02,
		0.459E+02,-0.310E+01,-0.165E+02,
		0.450E+02,-0.533E+01,-0.178E+02,
		0.421E+02, 0.766E+01,-0.190E+02,
		0.440E+02, 0.740E+01,-0.173E+02,
		0.453E+02, 0.763E+01,-0.157E+02,
		0.466E+02, 0.758E+01,-0.138E+02,
		0.481E+02, 0.714E+01,-0.114E+02,
		0.489E+02, 0.743E+01,-0.974E+01,
		0.498E+02, 0.785E+01,-0.808E+01,
		0.505E+02, 0.774E+01,-0.644E+01,
		-0.293E+02,-0.745E+01,-0.237E+02,
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
		0.510E+01,-0.100E+02,-0.383E+02,
		0.190E+01,-0.905E+01,-0.390E+02,
		-0.140E+01,-0.850E+01,-0.391E+02,
		-0.460E+01,-0.819E+01,-0.390E+02,
		-0.740E+01,-0.787E+01,-0.388E+02,
		-0.960E+01,-0.781E+01,-0.385E+02,
		-0.128E+02,-0.788E+01,-0.375E+02,
		-0.158E+02,-0.751E+01,-0.370E+02,
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
		-0.880E+01,-0.322E+01,-0.396E+02,
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
		0.860E+01, 0.134E+02,-0.364E+02,
		0.104E+02, 0.163E+02,-0.344E+02,
		0.127E+02, 0.191E+02,-0.322E+02,
		0.142E+02, 0.210E+02,-0.299E+02,
		0.159E+02, 0.234E+02,-0.277E+02,
		0.176E+02, 0.258E+02,-0.247E+02,
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
		0.320E+02, 0.137E+02,-0.250E+02,
		0.338E+02, 0.143E+02,-0.230E+02,
		0.349E+02, 0.154E+02,-0.210E+02,
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
		0.295E+02, 0.282E+02,-0.563E+01,
		0.314E+02, 0.269E+02,-0.777E+01,
		0.317E+02, 0.259E+02,-0.988E+01,
		0.324E+02, 0.241E+02,-0.116E+02,
		0.330E+02, 0.230E+02,-0.138E+02,
		0.343E+02, 0.213E+02,-0.153E+02,
		0.352E+02, 0.193E+02,-0.167E+02,
		0.359E+02, 0.177E+02,-0.181E+02,
		0.361E+02, 0.159E+02,-0.198E+02,
		-0.331E+02,-0.745E+01,-0.165E+02,
		-0.331E+02,-0.103E+02,-0.160E+02,
		-0.320E+02,-0.130E+02,-0.160E+02,
		-0.308E+02,-0.156E+02,-0.157E+02,
		-0.300E+02,-0.183E+02,-0.154E+02,
		-0.287E+02,-0.200E+02,-0.149E+02,
		-0.274E+02,-0.230E+02,-0.141E+02,
		-0.259E+02,-0.261E+02,-0.125E+02,
		-0.245E+02,-0.282E+02,-0.105E+02,
		-0.222E+02,-0.308E+02,-0.720E+01,
		-0.207E+02,-0.327E+02,-0.387E+01,
		-0.182E+02,-0.334E+02, 0.120E+01,
		-0.170E+02,-0.329E+02, 0.583E+01,
		-0.169E+02,-0.311E+02, 0.101E+02,
		-0.165E+02,-0.294E+02, 0.139E+02,
		-0.158E+02,-0.271E+02, 0.172E+02,
		-0.149E+02,-0.245E+02, 0.200E+02,
		-0.159E+02,-0.215E+02, 0.220E+02,
		-0.168E+02,-0.179E+02, 0.244E+02,
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
		-0.248E+02,-0.272E+02,-0.128E+02,
		-0.222E+02,-0.291E+02,-0.145E+02,
		-0.200E+02,-0.306E+02,-0.147E+02,
		-0.179E+02,-0.314E+02,-0.145E+02,
		-0.162E+02,-0.321E+02,-0.148E+02,
		-0.141E+02,-0.324E+02,-0.150E+02,
		-0.116E+02,-0.328E+02,-0.157E+02,
		-0.930E+01,-0.331E+02,-0.161E+02,
		-0.730E+01,-0.330E+02,-0.175E+02,
		-0.530E+01,-0.328E+02,-0.186E+02,
		-0.330E+01,-0.321E+02,-0.201E+02,
		-0.200E+00,-0.314E+02,-0.218E+02,
		0.200E+01,-0.307E+02,-0.226E+02,
		0.460E+01,-0.299E+02,-0.237E+02,
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
		0.336E+02,-0.226E+02,-0.133E+02,
		0.306E+02,-0.234E+02,-0.160E+02,
		0.288E+02,-0.236E+02,-0.182E+02,
		0.260E+02,-0.240E+02,-0.199E+02,
		0.230E+02,-0.250E+02,-0.210E+02,
		0.203E+02,-0.260E+02,-0.218E+02,
		0.176E+02,-0.270E+02,-0.221E+02,
		0.145E+02,-0.289E+02,-0.213E+02,
		0.117E+02,-0.298E+02,-0.220E+02,
		0.860E+01,-0.297E+02,-0.229E+02,
		0.640E+01,-0.299E+02,-0.231E+02,
		0.165E+02,-0.294E+02,-0.196E+02,
		0.183E+02,-0.297E+02,-0.172E+02,
		0.205E+02,-0.298E+02,-0.148E+02,
		0.228E+02,-0.293E+02,-0.130E+02,
		0.258E+02,-0.277E+02,-0.116E+02,
		0.286E+02,-0.269E+02,-0.100E+02,
		0.303E+02,-0.262E+02,-0.888E+01,
		0.284E+02,-0.268E+02, 0.795E+00,
		0.263E+02,-0.283E+02,-0.551E+00,
		0.239E+02,-0.298E+02,-0.163E+01,
		0.214E+02,-0.308E+02,-0.320E+01,
		0.194E+02,-0.319E+02,-0.400E+01,
		0.205E+02,-0.303E+02, 0.365E+01,
		0.193E+02,-0.307E+02, 0.176E+01,
		0.185E+02,-0.317E+02,-0.186E+00,
		0.180E+02,-0.322E+02,-0.244E+01,
		0.172E+02,-0.326E+02,-0.432E+01,
		0.153E+02,-0.329E+02,-0.601E+01,
		0.135E+02,-0.332E+02,-0.752E+01,
		0.114E+02,-0.338E+02,-0.891E+01,
		0.940E+01,-0.339E+02,-0.105E+02,
		0.720E+01,-0.340E+02,-0.110E+02,
		0.490E+01,-0.341E+02,-0.124E+02,
		0.320E+01,-0.342E+02,-0.132E+02,
		0.140E+01,-0.338E+02,-0.145E+02,
		-0.400E+00,-0.337E+02,-0.150E+02,
		-0.240E+01,-0.335E+02,-0.158E+02,
		-0.390E+01,-0.333E+02,-0.168E+02,
		-0.580E+01,-0.329E+02,-0.181E+02,
		-0.159E+02,-0.298E+02, 0.133E+02,
		-0.140E+02,-0.284E+02, 0.157E+02,
		-0.126E+02,-0.271E+02, 0.178E+02,
		-0.117E+02,-0.255E+02, 0.202E+02,
		-0.102E+02,-0.244E+02, 0.218E+02,
		-0.840E+01,-0.234E+02, 0.233E+02,
		-0.600E+01,-0.232E+02, 0.238E+02,
		-0.390E+01,-0.232E+02, 0.241E+02,
		-0.110E+01,-0.242E+02, 0.235E+02,
		0.210E+01,-0.232E+02, 0.234E+02,
		0.560E+01,-0.225E+02, 0.234E+02,
		0.880E+01,-0.224E+02, 0.226E+02,
		0.120E+02,-0.219E+02, 0.215E+02,
		0.152E+02,-0.211E+02, 0.208E+02,
		0.175E+02,-0.201E+02, 0.203E+02,
		0.204E+02,-0.185E+02, 0.201E+02,
		0.231E+02,-0.168E+02, 0.203E+02,
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
		0.463E+02,-0.139E+02,-0.853E+01,
		0.465E+02,-0.138E+02,-0.621E+01,
		0.467E+02,-0.144E+02,-0.354E+01,
		0.469E+02,-0.140E+02,-0.121E+01,
		0.469E+02,-0.136E+02, 0.673E-01,
		0.480E+02,-0.129E+02,-0.476E+00,
		0.493E+02,-0.108E+02,-0.144E+01,
		0.452E+02,-0.156E+02, 0.831E+00,
		0.435E+02,-0.171E+02, 0.539E+00,
		0.421E+02,-0.181E+02, 0.312E+00,
		0.410E+02,-0.191E+02, 0.591E+00,
		0.389E+02,-0.207E+02, 0.926E+00,
		0.362E+02,-0.221E+02, 0.191E+01,
		0.342E+02,-0.230E+02, 0.264E+01,
		0.326E+02,-0.237E+02, 0.389E+01,
		0.308E+02,-0.240E+02, 0.533E+01,
		0.292E+02,-0.242E+02, 0.663E+01,
		0.277E+02,-0.246E+02, 0.825E+01,
		0.308E+02,-0.225E+02, 0.861E+01,
		0.336E+02,-0.207E+02, 0.845E+01,
		0.353E+02,-0.201E+02, 0.850E+01,
		0.374E+02,-0.188E+02, 0.813E+01,
		0.385E+02,-0.188E+02, 0.706E+01,
		0.264E+02,-0.249E+02, 0.894E+01,
		0.244E+02,-0.254E+02, 0.107E+02,
		0.220E+02,-0.258E+02, 0.117E+02,
		0.198E+02,-0.262E+02, 0.124E+02,
		0.176E+02,-0.265E+02, 0.139E+02,
		0.150E+02,-0.268E+02, 0.150E+02,
		0.125E+02,-0.272E+02, 0.157E+02,
		0.900E+01,-0.274E+02, 0.175E+02,
		0.580E+01,-0.268E+02, 0.187E+02,
		0.300E+01,-0.266E+02, 0.196E+02,
		0.130E+01,-0.260E+02, 0.210E+02,
		-0.300E+00,-0.254E+02, 0.219E+02,
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
		0.124E+02,-0.707E+01, 0.284E+02,
		0.890E+01,-0.808E+01, 0.290E+02,
		0.620E+01,-0.757E+01, 0.295E+02,
		0.370E+01,-0.829E+01, 0.297E+02,
		0.100E+01,-0.843E+01, 0.302E+02,
		-0.150E+01,-0.912E+01, 0.302E+02,
		-0.340E+01,-0.861E+01, 0.301E+02,
		-0.590E+01,-0.957E+01, 0.302E+02,
		-0.820E+01,-0.992E+01, 0.299E+02,
		-0.102E+02,-0.103E+02, 0.297E+02,
		-0.124E+02,-0.116E+02, 0.293E+02,
		-0.139E+02,-0.133E+02, 0.278E+02,
		-0.159E+02,-0.153E+02, 0.262E+02,
		-0.164E+02,-0.168E+02, 0.253E+02,
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
		-0.363E+02, 0.124E+02,-0.179E+02,
		-0.363E+02, 0.152E+02,-0.174E+02,
		-0.354E+02, 0.173E+02,-0.158E+02,
		-0.354E+02, 0.192E+02,-0.137E+02,
		-0.359E+02, 0.208E+02,-0.126E+02,
		-0.359E+02, 0.227E+02,-0.107E+02,
		-0.354E+02, 0.247E+02,-0.962E+01,
		-0.354E+02, 0.263E+02,-0.718E+01,
		-0.354E+02, 0.279E+02,-0.518E+01,
		-0.354E+02, 0.298E+02,-0.341E+01,
		-0.354E+02, 0.323E+02,-0.233E+01,
		-0.354E+02, 0.352E+02,-0.157E+01,
		-0.343E+02, 0.379E+02, 0.951E+00,
		-0.341E+02, 0.395E+02, 0.347E+01,
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
		-0.317E+02, 0.426E+02, 0.584E+01,
		-0.285E+02, 0.436E+02, 0.849E+01,
		-0.252E+02, 0.444E+02, 0.961E+01,
		-0.224E+02, 0.446E+02, 0.932E+01,
		-0.200E+02, 0.445E+02, 0.994E+01,
		-0.178E+02, 0.447E+02, 0.110E+02,
		-0.159E+02, 0.447E+02, 0.130E+02,
		-0.138E+02, 0.445E+02, 0.136E+02,
		-0.106E+02, 0.440E+02, 0.148E+02,
		-0.750E+01, 0.440E+02, 0.155E+02,
		-0.530E+01, 0.437E+02, 0.161E+02,
		-0.300E+01, 0.435E+02, 0.167E+02,
		-0.100E+01, 0.431E+02, 0.165E+02,
		0.150E+01, 0.427E+02, 0.167E+02,
		0.380E+01, 0.414E+02, 0.176E+02,
		0.570E+01, 0.408E+02, 0.187E+02,
		0.780E+01, 0.397E+02, 0.195E+02,
		0.103E+02, 0.392E+02, 0.190E+02,
		0.128E+02, 0.388E+02, 0.181E+02,
		0.149E+02, 0.383E+02, 0.172E+02,
		0.172E+02, 0.372E+02, 0.167E+02,
		0.195E+02, 0.358E+02, 0.158E+02,
		0.216E+02, 0.350E+02, 0.157E+02,
		0.260E+01, 0.423E+02, 0.169E+02,
		0.370E+01, 0.429E+02, 0.144E+02,
		0.550E+01, 0.434E+02, 0.129E+02,
		0.680E+01, 0.440E+02, 0.105E+02,
		0.870E+01, 0.439E+02, 0.885E+01,
		0.106E+02, 0.438E+02, 0.695E+01,
		0.118E+02, 0.435E+02, 0.474E+01,
		-0.149E+02, 0.446E+02, 0.130E+02,
		-0.140E+02, 0.434E+02, 0.156E+02,
		-0.132E+02, 0.427E+02, 0.167E+02,
		-0.123E+02, 0.421E+02, 0.182E+02,
		-0.114E+02, 0.411E+02, 0.199E+02,
		-0.103E+02, 0.393E+02, 0.217E+02,
		-0.930E+01, 0.380E+02, 0.235E+02,
		-0.209E+02, 0.451E+02, 0.909E+01,
		-0.187E+02, 0.460E+02, 0.696E+01,
		-0.164E+02, 0.465E+02, 0.539E+01,
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
		-0.100E+02, 0.421E+02,-0.265E+02,
		-0.102E+02, 0.440E+02,-0.232E+02,
		-0.104E+02, 0.455E+02,-0.200E+02,
		-0.108E+02, 0.467E+02,-0.172E+02,
		-0.112E+02, 0.471E+02,-0.144E+02,
		-0.116E+02, 0.477E+02,-0.117E+02,
		-0.120E+02, 0.479E+02,-0.826E+01,
		-0.122E+02, 0.484E+02,-0.490E+01,
		-0.130E+02, 0.481E+02,-0.217E+01,
		-0.134E+02, 0.475E+02, 0.506E+00,
		-0.139E+02, 0.474E+02, 0.282E+01,
		-0.147E+02, 0.472E+02, 0.513E+01
	};

/*
Module functions
----------------
*/

int destroy_Sub_map(struct Sub_map **sub_map)
/*******************************************************************************
LAST MODIFIED : 25 July 2001

DESCRIPTION :
Destroy a sub map
==============================================================================*/
{
	int return_code;
	struct Map_frame *frame;
	struct Sub_map *the_sub_map;

	ENTER(destroy_Sub_map);
	if (sub_map&&(the_sub_map=*sub_map))
	{
		DEALLOCATE(the_sub_map->electrode_value);
		DEALLOCATE(the_sub_map->max_x);
		DEALLOCATE(the_sub_map->max_y);
		DEALLOCATE(the_sub_map->min_x);
		DEALLOCATE(the_sub_map->min_y);
		DEALLOCATE(the_sub_map->stretch_x);
		DEALLOCATE(the_sub_map->stretch_y);
		DEALLOCATE(the_sub_map->electrode_x);
		DEALLOCATE(the_sub_map->electrode_y);
		DEALLOCATE(the_sub_map->start_x);
		DEALLOCATE(the_sub_map->start_y);

		frame=&(the_sub_map->frame);
		DEALLOCATE(frame->contour_x);
		DEALLOCATE(frame->contour_y);
		DEALLOCATE(frame->pixel_values);
		if (frame->image)
		{
			DEALLOCATE(frame->image->data);
			XFree((char *)(frame->image));
		}

		/* free the Sub_map  structure */
		DEALLOCATE(the_sub_map);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Sub_map. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* destroy_Sub_map */

int map_destroy_Sub_maps(struct Map *map)
/*******************************************************************************
LAST MODIFIED : 7 September 2001

DESCRIPTION :
Destroy  Sub_maps in <map>. Assumes first_sub_map is the 1st sub map
in an array of number_of_sub_maps Sub_maps.
==============================================================================*/
{
	int i,number_of_sub_maps,return_code;
	struct Sub_map **sub_map,**first_sub_map;

	ENTER(map_destroy_Sub_maps);
	sub_map=(struct Sub_map **)NULL;
	first_sub_map=(struct Sub_map **)NULL;
	return_code=1;
	/*not necessarily an error if below untrue, as may be no sub_maps to destroy*/
	if ((number_of_sub_maps=map->number_of_sub_maps)&&(map->sub_map))
	{
		first_sub_map=map->sub_map;
		sub_map=first_sub_map;
		for(i=0;i<number_of_sub_maps;i++)
		{
			if (sub_map&&*sub_map)
			{
				return_code=destroy_Sub_map(sub_map);
				sub_map++;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"map_destroy_Sub_maps. Expected a Sub_map to destroy");
				return_code=0;
			}
		}
		map->number_of_sub_maps=0;
		DEALLOCATE(map->sub_map);

	}
	LEAVE;
	
	return (return_code);
}/* map_destroy_Sub_maps */

int map_add_Sub_map(struct Map *map,struct Sub_map *sub_map)
/*******************************************************************************
LAST MODIFIED : 7 September  2001

DESCRIPTION :
Add <sub_map> to the list in <map>
==============================================================================*/
{
	int return_code;
	struct Sub_map **new_sub_maps;

	ENTER(map_add_Sub_map);
	return_code=1;
	if (map&&sub_map)
	{
		map->number_of_sub_maps++;
		REALLOCATE(new_sub_maps,map->sub_map,struct Sub_map *,map->number_of_sub_maps);
		map->sub_map=new_sub_maps;
		map->sub_map[map->number_of_sub_maps-1]=sub_map;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_add_Sub_map. invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_add_Sub_map */

struct Sub_map *create_Sub_map(int number_of_drawn_regions,
	int number_of_electrodes,int x_offset,int y_offset,int height,int width,
	float frame_time,int use_potential_time)
/*******************************************************************************
LAST MODIFIED : 17 October 2001

DESCRIPTION :
Create a sub map.
==============================================================================*/
{
	struct Sub_map *sub_map;

	ENTER(create_Sub_map);
	sub_map=(struct Sub_map *)NULL;
	if ((number_of_drawn_regions>0)&&(number_of_electrodes>0))
	{
		if (ALLOCATE(sub_map,struct Sub_map,1))
		{
			sub_map->height=height;
			sub_map->width=width;
			sub_map->x_offset=x_offset;
			sub_map->y_offset=y_offset;
			sub_map->frame_time=frame_time;
			sub_map->use_potential_time=use_potential_time;
			sub_map->maximum_value=0;
			sub_map->frame.maximum_screen_x= -1;
			sub_map->frame.maximum_screen_y= -1;
			sub_map->minimum_value=0;
			sub_map->frame.minimum_screen_x= -1;
			sub_map->frame.minimum_screen_y= -1;
			sub_map->frame.pixel_values=(float *)NULL;
			sub_map->frame.image=(XImage *)NULL;
			sub_map->frame.contour_x=(short int *)NULL;
			sub_map->frame.contour_y=(short int *)NULL;
			ALLOCATE(sub_map->electrode_value,float,number_of_electrodes);
			ALLOCATE(sub_map->max_x,float,number_of_drawn_regions);
			ALLOCATE(sub_map->max_y,float,number_of_drawn_regions);
			ALLOCATE(sub_map->min_x,float,number_of_drawn_regions);
			ALLOCATE(sub_map->min_y,float,number_of_drawn_regions);
			ALLOCATE(sub_map->stretch_x,float,number_of_drawn_regions);
			ALLOCATE(sub_map->stretch_y,float,number_of_drawn_regions);
			ALLOCATE(sub_map->electrode_x,int,number_of_electrodes);
			ALLOCATE(sub_map->electrode_y,int,number_of_electrodes);
			ALLOCATE(sub_map->start_x,int,number_of_drawn_regions);
			ALLOCATE(sub_map->start_y,int,number_of_drawn_regions);
			if (!((sub_map->electrode_value)&&(sub_map->max_x)
				&&(sub_map->max_y)&&(sub_map->min_x)&&(sub_map->min_y)&&
				(sub_map->stretch_x)&&(sub_map->stretch_y)&&(sub_map->electrode_x)&&
				(sub_map->electrode_y)&&(sub_map->start_x)&&(sub_map->start_y)))
			{
				display_message(ERROR_MESSAGE,
					"create_Sub_map.  Out of memory for contents");
				destroy_Sub_map(&sub_map);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Sub_map. Out of memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Sub_map. Invalid arguments");
	}
	LEAVE;
	
	return (sub_map);
} /* create_Sub_map*/

#if defined (UNEMAP_USE_3D)
static int set_mapping_FE_node_coord_values(struct FE_node *node,
	struct FE_field *position_field,
	int coords_comp_0_num_versions,int coords_comp_1_num_versions,
	int coords_comp_2_num_versions,FE_value *coords_comp_0,
	FE_value *coords_comp_1,FE_value *coords_comp_2,enum Region_type region_type)
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
sets a node's values storage with the coordinate system component versions and
values, <coords_comp_X_num_versions>,<coords_comp_X>, using <field_order_info>
==============================================================================*/
{
	enum FE_nodal_value_type *component_value_types;
	FE_value *value;
	int i,j,number_of_derivatives,return_code;
	struct FE_field_component component;

	ENTER(set_mapping_FE_node_coord_values)
	if (node&&position_field&&coords_comp_0&&coords_comp_1&&
		coords_comp_2)
	{
		return_code=1;
		switch (region_type)
		{
			case SOCK:/*lambda,mu,theta */
			case TORSO:/*theta,r , z */
			{
				/* 1st field contains coords, possibly more than one version */
				component.field=position_field;
				component.number = 0;
				number_of_derivatives=get_FE_node_field_component_number_of_derivatives(node,
					position_field,0);
				component_value_types=get_FE_node_field_component_nodal_value_types(node,
					position_field,0);
				value=coords_comp_0;
				for(i=0;i<coords_comp_0_num_versions;i++)
				{
					/*derivatives+1 as value then derivatives */
					for(j=0;j<number_of_derivatives+1;j++)
					{
						return_code=(return_code&&set_FE_nodal_FE_value_value(node,&component,i,
							component_value_types[j],/*time*/0,*value)); /*lambda,theta,x*/
						value++;
					}
				}
				DEALLOCATE(component_value_types);
				component.number = 1;
				for(i=0;i<coords_comp_1_num_versions;i++)
				{
					return_code=(return_code&&set_FE_nodal_FE_value_value(node,&component,i,
						FE_NODAL_VALUE,/*time*/0,coords_comp_1[i])); /*mu,r,y*/
				}
				component.number = 2;
				for(i=0;i<coords_comp_2_num_versions;i++)
				{
					return_code=(return_code&&set_FE_nodal_FE_value_value(node,&component,i,
						FE_NODAL_VALUE,/*time*/0,coords_comp_2[i])); /*theta,z,z */
				}
			} break;
			case PATCH: /* x,y */
			{
				/* 1st field contains coords, possibly more than one version */
				component.field=position_field;
				component.number = 0;
				for(i=0;i<coords_comp_0_num_versions;i++)
				{
					return_code=(return_code&&set_FE_nodal_FE_value_value(node,&component,i,
						FE_NODAL_VALUE,/*time*/0,coords_comp_0[i])); /*x*/
				}
				component.number = 1;
				for(i=0;i<coords_comp_1_num_versions;i++)
				{
					return_code=(return_code&&set_FE_nodal_FE_value_value(node,&component,i,
						FE_NODAL_VALUE,/*time*/0,coords_comp_1[i])); /*y*/
				}
				component.number = 2;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"set_mapping_FE_node_coord_values."
					"  Invalid region_type");
				node=(struct FE_node *)NULL;
			} break;
		}	/* switch () */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_mapping_FE_node_coord_values. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_mapping_FE_node_coord_values */

#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int set_mapping_FE_node_fit_values(struct FE_node *node,
	struct FE_field *fit_field,int component_number,
	FE_value f,FE_value dfdx,FE_value dfdy,FE_value d2fdxdy)
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
Sets a node's values storage with the values  potential<f> and derivatives
<dfdx>, <dfdy>,<d2fdxdy> using  <field_order_info.>
==============================================================================*/
{
	int return_code;
	struct FE_field_component component;

	ENTER(set_mapping_FE_node_fit_values);
	return_code =1;
	if (node&&fit_field)
	{
		component.field=fit_field;
		component.number=component_number;
		return_code=(
			set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,/*time*/0,f)&&
			set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_D_DS1,/*time*/0,dfdx)&&
			set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_D_DS2,/*time*/0,dfdy)&&
			set_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_D2_DS1DS2,/*time*/0,d2fdxdy));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_mapping_FE_node_fit_values. Invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
}/* set_mapping_FE_node_fit_values */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct FE_node *create_and_set_mapping_FE_node(struct FE_node *template_node,
	struct MANAGER(FE_node) *node_manager,
	enum Region_type	region_type,int node_number,
	struct FE_field_order_info *field_order_info,
	int coords_comp_0_num_versions,int coords_comp_1_num_versions,
	int coords_comp_2_num_versions,FE_value *coords_comp_0,FE_value *coords_comp_1,
	FE_value *coords_comp_2,FE_value f,FE_value dfdx,FE_value dfdy,FE_value d2fdxdy)
/*******************************************************************************
LAST MODIFIED : 30 July 1999

DESCRIPTION :
sets a node's values storage with the coordinate system component versions and values,
<coords_comp_X_num_versions>,<coords_comp_X>, potential<f> and derivatives <dfdx>,
<dfdy>,<d2fdxdy> using the template node created in create_mapping_template_node,
and the field_order_info.
==============================================================================*/
{
	struct FE_node *node;
	struct FE_field *field;

	ENTER(create_and_set_mapping_FE_node);
	if (template_node&&node_manager&&field_order_info&&coords_comp_0&&
		coords_comp_1&&coords_comp_2)
	{
		node=(struct FE_node *)NULL;
		field=(struct FE_field *)NULL;
		if (node=CREATE(FE_node)(node_number,template_node))
		{
			/*	set up the type dependent values	*/
			/* 1st field is position field*/
			field=get_FE_field_order_info_field(field_order_info,0);
			set_mapping_FE_node_coord_values(node,field,
				coords_comp_0_num_versions,coords_comp_1_num_versions,
				coords_comp_2_num_versions,coords_comp_0,coords_comp_1,coords_comp_2,
				region_type);
			/*Set fields which are identical in all nodes*/
			/* 2nd field contains fit */
			field=get_FE_field_order_info_field(field_order_info,1);
			set_mapping_FE_node_fit_values(node,field,0/*component_number*/,f,dfdx,dfdy,d2fdxdy);
			if (FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
				cm_node_identifier)(node_number,node_manager))
			{
				display_message(ERROR_MESSAGE,"create_and_set_mapping_FE_node. Node already exists!");
				node=(struct FE_node *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"create_and_set_mapping_FE_node.  Error creating node");
			node=(struct FE_node *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_and_set_mapping_FE_node.  Invalid argument(s)");
		node=(struct FE_node *)NULL;
	}
	LEAVE;
	
	return (node);
}/* create_and_set_mapping_FE_node*/
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct FE_field_order_info *create_mapping_fields(
	enum Region_type region_type,FE_value focus,struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
Creates the mapping fields, returns them in a FE_field_order_info.
==============================================================================*/
{
#define NUM_MAPPING_FIELDS 2 /* position_field,fit_field*/

	struct MANAGER(FE_field) *fe_field_manager;
	struct FE_field_order_info *the_field_order_info =
		(struct FE_field_order_info *)NULL;
	int success, string_length;
	char *fit_point_name,*map_type_str;
	struct Coordinate_system coordinate_system;
	struct FE_field *position_field,*potential_field;
	struct FE_time *fe_time;

	char *patch_str = "patch";
	char *sock_str = "sock";
	char *torso_str = "torso";
	char *patch_fit_point_component_names[2]=
	{
		"x","y"
	};
	char *sock_fit_point_component_names[3]=
	{
		"lambda","mu","theta"
	};
	char *torso_fit_point_component_names[3]=
	{
		"r","theta","z"
	};
	char *position_str = "_fit_point_position";

	if (package)
	{
		success = 1;
		fit_point_name = (char *)NULL;
		fe_field_manager=get_unemap_package_FE_field_manager(package);
		fe_time=get_unemap_package_FE_time(package);
		switch (region_type)
		{
			case SOCK:
			{
				map_type_str = sock_str;
			} break;
			case TORSO:
			{
				map_type_str = torso_str;
			} break;
			case PATCH:
			{
				map_type_str = patch_str;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"create_mapping_fields. Bad region type");
				success=0;
			} break;
		}

		string_length = strlen(map_type_str);
		string_length += strlen(position_str);
		string_length++;
		if (ALLOCATE(fit_point_name,char,string_length))
		{
			strcpy(fit_point_name,map_type_str);
			strcat(fit_point_name,position_str);
			/* create the FE_field_order_info for all the fields */
			switch (region_type)
			{
				case SOCK:
				{
					if (the_field_order_info = CREATE(FE_field_order_info)())
					{
						/* set up the info needed to create the  fit point position field */
						coordinate_system.type=PROLATE_SPHEROIDAL;
						coordinate_system.parameters.focus = focus;
						/* create the fit point position field, add it to the node and
							create */
						if (!(position_field=get_FE_field_manager_matched_field(
							fe_field_manager,fit_point_name,
							GENERAL_FE_FIELD,fe_time,/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
							&coordinate_system,FE_VALUE_VALUE,
							/*number_of_components*/3,sock_fit_point_component_names,
							/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL)))
						{
							display_message(ERROR_MESSAGE,
								"create_mapping_fields.Could not retrieve sock fit point"
								" position field");
							success=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_mapping_fields.Could not create FE_field_order_info"
							" position field");
						success=0;
					}
				} break;
				case TORSO:
				{
					if (the_field_order_info = CREATE(FE_field_order_info)())
					{
						/* set up the info needed to create the  fit point position field */
						coordinate_system.type=CYLINDRICAL_POLAR;
						/* create the fit point position field, add it to the node */
						if (!(position_field=get_FE_field_manager_matched_field(
							fe_field_manager,fit_point_name,
							GENERAL_FE_FIELD,fe_time,/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
							&coordinate_system,FE_VALUE_VALUE,
							/*number_of_components*/3,torso_fit_point_component_names,
							/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL)))
						{
							display_message(ERROR_MESSAGE,
								"create_mapping_fields. Could not torso retrieve fit point"
								" position field");
							success=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_mapping_fields.Could not create FE_field_order_info"
							" position field");
						success=0;
					}
				} break;
				case PATCH:
				{
					if (the_field_order_info = CREATE(FE_field_order_info)())
					{
						/* set up the info needed to create the  fit point position field */
						coordinate_system.type=RECTANGULAR_CARTESIAN;
						/* create the fit point position field, add it to the node */
						if (!(position_field=get_FE_field_manager_matched_field(
							fe_field_manager,fit_point_name,
							GENERAL_FE_FIELD,fe_time,/*indexer_field*/(struct FE_field *)NULL,
							/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
							&coordinate_system,FE_VALUE_VALUE,
							/*number_of_components*/2,patch_fit_point_component_names,
							/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
							(struct FE_field_external_information *)NULL)))
						{
							display_message(ERROR_MESSAGE,
								"create_mapping_fields. Could not retrieve patch "
								"fit point position field");
							success=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_mapping_fields.Could not create FE_field_order_info"
							" position field");
						success=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"create_mapping_fields. Bad region type");
					success=0;
				} break;
			} /* switch */
			if (success)
			{
				success =
					add_FE_field_order_info_field(the_field_order_info, position_field);
			}
			/* create fields common to all nodes*/
			if (success)
			{
				if (potential_field=get_unemap_package_map_fit_field(package))
				{
					success = add_FE_field_order_info_field(the_field_order_info,
						potential_field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_mapping_fields. Could not retrieve potential_value field");
					success = 0;
				}
			} /* if (success)*/
			DEALLOCATE(fit_point_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_mapping_fields. Could not allocate memory for "
				"fit_point_name");
			success =0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_mapping_fields. Invalid arguments");
	}
	LEAVE;
	
	return (the_field_order_info);
}/* create_mapping_fields */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct FE_node *create_mapping_template_node(enum Region_type region_type,
	struct FE_field_order_info *field_order_info,
	struct Unemap_package *package,int coords_comp_0_num_versions,
	int coords_comp_1_num_versions,int coords_comp_2_num_versions)
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Creates and returns a mapping template node for interpolation_function_to_node_group.
<region_type>==PATCH has 2D position field, others 3D
==============================================================================*/
{
	struct FE_node *node;
	int success;
	struct FE_field *position_field,*fit_field;
	struct FE_node_field_creator *node_field_creator;

	ENTER(create_mapping_template_node);
	if (package)
	{
		success = 1;
		/* create the node */
		if (node=CREATE(FE_node)(0,(struct FE_node *)NULL))
		{
			/* first field in field_order_info is fit_point_position_field */
			position_field=get_FE_field_order_info_field(field_order_info,0);
			switch (region_type)
			{
				case PATCH:
				{
					node_field_creator = CREATE(FE_node_field_creator)(
						/* number_of_components*/2);
					/* The default components are linear */
					/* Define versions */
					FE_node_field_creator_define_versions(node_field_creator,
						/*component_number*/0, coords_comp_0_num_versions);
					FE_node_field_creator_define_versions(node_field_creator,
						/*component_number*/1, coords_comp_1_num_versions);
					success=define_FE_field_at_node(node,position_field,
						(struct FE_time_version *)NULL,
						node_field_creator);
					DESTROY(FE_node_field_creator)(&node_field_creator);
				} break;
				case TORSO:
				{
					node_field_creator = CREATE(FE_node_field_creator)(
						/* number_of_components*/3);
					/* The default components are linear, make component 0 bicubic */
					FE_node_field_creator_define_derivative(node_field_creator,
						/*component_number*/0, FE_NODAL_D_DS1);
					FE_node_field_creator_define_derivative(node_field_creator,
						/*component_number*/0, FE_NODAL_D_DS2);
					FE_node_field_creator_define_derivative(node_field_creator,
						/*component_number*/0, FE_NODAL_D2_DS1DS2);
					/* Define versions */
					FE_node_field_creator_define_versions(node_field_creator,
						/*component_number*/0, coords_comp_0_num_versions);
					FE_node_field_creator_define_versions(node_field_creator,
						/*component_number*/1, coords_comp_1_num_versions);
					FE_node_field_creator_define_versions(node_field_creator,
						/*component_number*/2, coords_comp_2_num_versions);
					success=define_FE_field_at_node(node,position_field,
						(struct FE_time_version *)NULL,
						node_field_creator);
					DESTROY(FE_node_field_creator)(&node_field_creator);
				} break;
				default:
				{
					node_field_creator = CREATE(FE_node_field_creator)(
						/* number_of_components*/3);
					/* The default components are linear */
					/* Define versions */
					FE_node_field_creator_define_versions(node_field_creator,
						/*component_number*/0, coords_comp_0_num_versions);
					FE_node_field_creator_define_versions(node_field_creator,
						/*component_number*/1, coords_comp_1_num_versions);
					FE_node_field_creator_define_versions(node_field_creator,
						/*component_number*/2, coords_comp_2_num_versions);
					success=define_FE_field_at_node(node,position_field,
						(struct FE_time_version *)NULL,
						node_field_creator);
					DESTROY(FE_node_field_creator)(&node_field_creator);
				} break;
			}
			if (success)
			{
				/* 2nd field in field_order_info is fit_field */
				fit_field=get_FE_field_order_info_field(field_order_info,1);
				node_field_creator = CREATE(FE_node_field_creator)(
					/* number_of_components*/1);
				/* The default components are linear, make component 0 bicubic */
				FE_node_field_creator_define_derivative(node_field_creator,
					/*component_number*/0, FE_NODAL_D_DS1);
				FE_node_field_creator_define_derivative(node_field_creator,
					/*component_number*/0, FE_NODAL_D_DS2);
				FE_node_field_creator_define_derivative(node_field_creator,
					/*component_number*/0, FE_NODAL_D2_DS1DS2);
				success=define_FE_field_at_node(node,fit_field,
					(struct FE_time_version *)NULL,
					node_field_creator);
				DESTROY(FE_node_field_creator)(&node_field_creator);
			} /* if (success)*/
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_mapping_template_node.  Could not create node");
			success=0;
		}
		if (!success)
		{
			DESTROY(FE_node)(&node);
			node = (struct FE_node *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_mapping_template_node. Invalid arguments");
		node = (struct FE_node *)NULL;
	}
	LEAVE;
	
	return (node);
}/* create_mapping_template_node */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct GROUP(FE_node) *interpolation_function_to_node_group(
	struct FE_node_order_info **the_node_order_info,
	struct FE_field_order_info *field_order_info,
	struct Interpolation_function *function,struct Unemap_package *package,
	char *name,FE_value sock_lambda,FE_value torso_major_r,
	FE_value torso_minor_r,FE_value patch_z,struct Region *region)
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Constructs a node group from the <function> and <package>
function->x_mesh has number_of_columns+1 entries.
function->y_mesh has number_of_rows+1 entries.
function->f has (number_of_columns+1)*(has number_of_rows+1) entries

x_mesh and y_mesh form a rectangular grid.
if c =number_of_columns,r= number_of_rows:

.x_mesh[0],y_mesh[r]        .x_mesh[c],y_mesh[r]
	f[r(c+1)]                   f[(r+1)*(c+1)-1]



f[0]                         f[c]
.x_mesh[0],y_mesh[0]        .x_mesh[c],y_mesh[0] <-For sock, this row forms 1 node
^                           ^
|                           |
For sock and torso, these 2 columns are the same, and form one column of (r+1) nodes

i.e sock is a hemisphere, torso is a cylinder.

==============================================================================*/
{
	enum Region_type region_type;
	struct FE_field *map_position_field,*map_fit_field;
	struct FE_node *template_node,*node;
	struct FE_node_order_info *node_order_info;
	FE_value dfdx,dfdy,d2fdxdy,f,lambda,mu,r_value_and_derivatives[4],theta,x,y,z;
	int count,f_index,i,j,last_node_number,node_number,number_of_columns,number_of_nodes,
		number_of_rows;
	struct GROUP(FE_node) *interpolation_node_group;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct Map_3d_package *map_3d_package;
#if !defined(ROUND_TORSO)
	FE_value c,dr_dt,/*dt_ds,*/dt_dxi,r,s,t,torso_x,torso_y;
#endif /* !defined(ROUND_TORSO)*/
	ENTER(interpolation_function_to_node_group);
#if defined(ROUND_TORSO)
	USE_PARAMETER(torso_minor_r);
#endif /* !defined(ROUND_TORSO)*/
	node_order_info =(struct FE_node_order_info *)NULL;
	interpolation_node_group =(struct GROUP(FE_node) *)NULL;
	template_node = (struct FE_node *)NULL;
	map_position_field= (struct FE_field *)NULL;
	map_fit_field= (struct FE_field *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	if (function&&package&&name)
	{
		last_node_number=1;
		region_type=function->region_type;
		number_of_rows=function->number_of_rows;
		number_of_columns=function->number_of_columns;
		element_group_manager=get_unemap_package_element_group_manager(package);
		node_manager=get_unemap_package_node_manager(package);
		element_manager=get_unemap_package_element_manager(package);
		data_group_manager=get_unemap_package_data_group_manager(package);
		node_group_manager= get_unemap_package_node_group_manager(package);

		MANAGER_BEGIN_CACHE(FE_node)(node_manager);
		/* make the node group */
		interpolation_node_group=make_node_and_element_and_data_groups(
			node_group_manager,node_manager,element_manager,element_group_manager,
			data_group_manager,name);
		MANAGED_GROUP_BEGIN_CACHE(FE_node)(interpolation_node_group);
		/* create the node_order_info */
		switch (region_type)
		{
			case SOCK:
			{
				number_of_nodes = (number_of_rows*number_of_columns)+1;
			} break;
			case TORSO:
			{
				number_of_nodes = (number_of_rows+1)*number_of_columns;
			} break;
			case PATCH:
			{
				number_of_nodes = (number_of_rows+1)*(number_of_columns+1);
			} break;
		}
		if (node_order_info=CREATE(FE_node_order_info)(number_of_nodes))
		{
			/* create the template node, based upon the region_type */
			switch (region_type)
			{
				case SOCK:
				{
					node_number = get_next_FE_node_number(node_manager,last_node_number);
					/* Do apex, one node, lots of number_of_columns versions of theta */
					if (template_node)
					{
						DESTROY(FE_node)(&template_node);
					}
					template_node = create_mapping_template_node(SOCK,field_order_info,
						package,1,1,number_of_columns);
					theta = function->x_mesh[0];
					mu = 0;
					lambda = sock_lambda;
					f=function->f[0];
					dfdx=function->dfdx[0];
					dfdy=function->dfdy[0];
					d2fdxdy=function->d2fdxdy[0];
					if (!(node=create_and_set_mapping_FE_node(template_node,node_manager,
						region_type,node_number,field_order_info,1,1,number_of_columns,
						&lambda,&mu,function->x_mesh,f,dfdx,dfdy,d2fdxdy)))
					{
						display_message(ERROR_MESSAGE,"interpolation_function_to_node_group."
							" Error creating node");
					}
					if (node)
					{
						if (ADD_OBJECT_TO_MANAGER(FE_node)(node,node_manager))
						{
							if (ADD_OBJECT_TO_GROUP(FE_node)(node,interpolation_node_group))
							{
								set_FE_node_order_info_node(node_order_info,0,
									node);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"interpolation_function_to_node_group."
									" Could not add node to node_group");
								REMOVE_OBJECT_FROM_GROUP(FE_node)(node,interpolation_node_group);
								REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
								node=(struct FE_node *)NULL;
							}
						}/* if (!ADD_OBJECT_TO_MANAGER(FE_node) */
						else
						{
							display_message(ERROR_MESSAGE,
								"interpolation_function_to_node_group. Could not add node to node_manager");
							REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
							node=(struct FE_node *)NULL;
						}
					}/*	if (node) */
					last_node_number = node_number;
					count=1;
					if (template_node)
					{
						DESTROY(FE_node)(&template_node);
					}
					template_node = create_mapping_template_node(region_type,field_order_info,
						package,1,1,1);
					while(count<number_of_nodes)
					{
						/* done apex, so skip first row */
						for(i=1;i<number_of_rows+1;i++)
						{
							/* skip last column, as same as first*/
							for(j=0;j<number_of_columns;j++)
							{
								node_number = get_next_FE_node_number(node_manager,last_node_number);
								theta=function->x_mesh[j];
								mu=function->y_mesh[i];
								lambda=sock_lambda;
								f_index=j+(i*(number_of_columns+1));
								f=function->f[f_index];
								dfdx=function->dfdx[f_index];
								dfdy=function->dfdy[f_index];
								d2fdxdy=function->d2fdxdy[f_index];
								if (!(node=create_and_set_mapping_FE_node(template_node,node_manager,region_type,
									node_number,field_order_info,1,1,1,&lambda,&mu,&theta,f,
									dfdx,dfdy,d2fdxdy)))
								{
									display_message(ERROR_MESSAGE,"interpolation_function_to_node_group."
										" Error creating node");
								}
								last_node_number = node_number;
								if (node)
								{
									if (ADD_OBJECT_TO_MANAGER(FE_node)(node,node_manager))
									{
										if (ADD_OBJECT_TO_GROUP(FE_node)(node,interpolation_node_group))
										{
											set_FE_node_order_info_node(node_order_info,count,
												node);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"interpolation_function_to_node_group."
												" Could not add node to node_group");
											REMOVE_OBJECT_FROM_GROUP(FE_node)(node,interpolation_node_group);
											REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
											node=(struct FE_node *)NULL;
										}
									}/* if (!ADD_OBJECT_TO_MANAGER(FE_node) */
									else
									{
										display_message(ERROR_MESSAGE,
											"interpolation_function_to_node_group. "
											"Could not add node to node_manager");
										REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
										node=(struct FE_node *)NULL;
									}
								}/*	if (node) */
								count++;
							} /* for(j= */
						} /* for(i= */
					} /* while(count */
				} break;
				case TORSO:
				{
					count=0;
					if (template_node)
					{
						DESTROY(FE_node)(&template_node);
					}
					template_node = create_mapping_template_node(TORSO,field_order_info,
						package,1,1,1);
					r_value_and_derivatives[0]=0;
					r_value_and_derivatives[1]=0;
					r_value_and_derivatives[2]=0;
					r_value_and_derivatives[3]=0;
					while(count<number_of_nodes)
					{
						/* do all rows */
						for(i=0;i<number_of_rows+1;i++)
						{
							/* skip last column, as same as first*/
							for(j=0;j<number_of_columns;j++)
							{
								node_number = get_next_FE_node_number(node_manager,last_node_number);
								theta=function->x_mesh[j];
								z=function->y_mesh[i];
								/* ROUND_TORSO defined in rig_node.h */
#if defined (ROUND_TORSO) /*FOR AJP*/
								r_value_and_derivatives[0]= torso_major_r;
#else
								t=function->x_mesh[j];
								c=cos(t);
								s=sin(t);
								torso_x=torso_major_r*c;
								torso_y=torso_minor_r*s;
#if defined (NEW_CODE2)
								theta=atan2(torso_y,torso_x);
#else /* defined (NEW_CODE2) */
								theta=t;
#endif /* defined (NEW_CODE2) */
								r=sqrt((torso_x*torso_x)+(torso_y*torso_y));
								r_value_and_derivatives[0]= r;
								dr_dt= (s*c*(torso_major_r+torso_minor_r)*
									(torso_minor_r-torso_major_r))/r;
#if defined (NEW_CODE)
								dt_ds=1/sqrt(torso_major_r*torso_major_r*s*s+
									torso_minor_r*torso_minor_r*c*c);
								r_value_and_derivatives[1]=dr_dt*dt_ds;
#else /* defined (NEW_CODE) */
								if (0==j)
								{
									dt_dxi=((function->x_mesh)[1]-
										(function->x_mesh)[number_of_columns-1])/2;
								}
								else
								{
									if (j==number_of_columns-1)
									{
										dt_dxi=(8*atan(1)+(function->x_mesh)[0]-
											(function->x_mesh)[number_of_columns-2])/2;
									}
									else
									{
										dt_dxi=((function->x_mesh)[j+1]-
											(function->x_mesh)[j-1])/2;
									}
								}
								r_value_and_derivatives[1]=dr_dt*dt_dxi;
#endif /* defined (NEW_CODE) */
								r_value_and_derivatives[2]= 0;
								r_value_and_derivatives[3]= 0;
#if defined (DEBUG)
								if (0==i)
								{
									printf("j= %d ",j);
									printf("torso_major_r= %g ",torso_major_r);
									printf("torso_minor_r= %g ",torso_minor_r);
									printf("theta= %g\n",theta);
									printf("c= %g ",c);
									printf("s= %g ",s);
									printf("torso_x= %g\n",torso_x);
									printf("torso_y= %g ",torso_y);
									printf("r= %g\n",r);
									printf("dr_dt=%g, dt_ds=%g, dr_ds=%g\n",
										dr_dt,dt_ds,r_value_and_derivatives[1]);
								}
#endif /* defined (DEBUG)	*/
#endif /* defined (ROUND_TORSO) */
								f_index=j+(i*(number_of_columns+1));
								f=function->f[f_index];
								dfdx=function->dfdx[f_index];
								dfdy=function->dfdy[f_index];
								d2fdxdy=function->d2fdxdy[f_index];
								if (!(node=create_and_set_mapping_FE_node(template_node,node_manager,region_type,
									node_number,field_order_info,1,1,1,r_value_and_derivatives,&theta,&z,f,dfdx,
									dfdy,d2fdxdy)))
								{
									display_message(ERROR_MESSAGE,"interpolation_function_to_node_group."
										" Error creating node");
								}
								last_node_number = node_number;
								if (node)
								{
									if (ADD_OBJECT_TO_MANAGER(FE_node)(node,node_manager))
									{
										if (ADD_OBJECT_TO_GROUP(FE_node)(node,interpolation_node_group))
										{
											set_FE_node_order_info_node(node_order_info,count,
												node);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"interpolation_function_to_node_group. Could not add "
												"node to node_group");
											REMOVE_OBJECT_FROM_GROUP(FE_node)(node,interpolation_node_group);
											REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
											node=(struct FE_node *)NULL;
										}
									}/* if (!ADD_OBJECT_TO_MANAGER(FE_node) */
									else
									{
										display_message(ERROR_MESSAGE,
											"interpolation_function_to_node_group. Could not add node "
											"to node_manager");
										REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
										node=(struct FE_node *)NULL;
									}
								}/*	if (node) */
								count++;
							} /* for(j= */
						} /* for(i= */
					} /* while(count */
				} break;
				case PATCH:
				{
					count=0;
					if (template_node)
					{
						DESTROY(FE_node)(&template_node);
					}
					template_node = create_mapping_template_node(PATCH,field_order_info,
						package,1,1,1);
					while(count<number_of_nodes)
					{
						/* do all rows */
						for(i=0;i<number_of_rows+1;i++)
						{
							/* do all columns */
							for(j=0;j<number_of_columns+1;j++)
							{
								node_number = get_next_FE_node_number(node_manager,last_node_number);
								x=function->x_mesh[j];
								y=function->y_mesh[i];
								z=patch_z;
								f=function->f[count];
								dfdx=function->dfdx[count];
								dfdy=function->dfdy[count];
								d2fdxdy=function->d2fdxdy[count];
								if (!(node=create_and_set_mapping_FE_node(template_node,node_manager,
									region_type,node_number,field_order_info,1,1,1,&x,&y,&z,f,dfdx,dfdy,
									d2fdxdy)))
								{
									display_message(ERROR_MESSAGE,"interpolation_function_to_node_group."
										" Error creating node");
								}
								last_node_number = node_number;
								if (node)
								{
									if (ADD_OBJECT_TO_MANAGER(FE_node)(node,node_manager))
									{
										if (ADD_OBJECT_TO_GROUP(FE_node)(node,interpolation_node_group))
										{
											set_FE_node_order_info_node(node_order_info,count,
												node);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"interpolation_function_to_node_group. Could not add node to"
												" node_group");
											REMOVE_OBJECT_FROM_GROUP(FE_node)(node,interpolation_node_group);
											REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
											node=(struct FE_node *)NULL;
										}
									}/* if (!ADD_OBJECT_TO_MANAGER(FE_node) */
									else
									{
										display_message(ERROR_MESSAGE,
											"interpolation_function_to_node_group."
											" Could not add node to node_manager");
										REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,node_manager);
										node=(struct FE_node *)NULL;
									}
								}/*	if (node) */
								count++;
							} /* for(j= */
						} /* for(i= */
					} /* while(count */
				} break; /* case PATCH:*/
			} /* switch (region_type) */
			/* 1st field is map position*/
			map_position_field=get_FE_field_order_info_field(field_order_info,0);
			/* 2nd field is map fit*/
			map_fit_field=get_FE_field_order_info_field(field_order_info,1);
			/* store map info to use next time and so can free it */
			/* Create the new Map_3d_package*/
			map_3d_package=CREATE(Map_3d_package)(
				get_unemap_package_FE_field_manager(package),
				get_unemap_package_element_group_manager(package),
				get_unemap_package_data_manager(package),
				get_unemap_package_data_group_manager(package),
				get_unemap_package_node_manager(package),
				get_unemap_package_node_group_manager(package),
				get_unemap_package_element_manager(package),
				get_unemap_package_Computed_field_manager(package));
			set_map_3d_package_number_of_map_rows(map_3d_package,number_of_rows);
			set_map_3d_package_number_of_map_columns(map_3d_package,number_of_columns);
			set_map_3d_package_fit_name(map_3d_package,name);
			set_map_3d_package_node_order_info(map_3d_package,node_order_info);
			set_map_3d_package_position_field(map_3d_package,map_position_field);
			set_map_3d_package_fit_field(map_3d_package,map_fit_field);
			set_map_3d_package_node_group(map_3d_package,interpolation_node_group);
			set_Region_map_3d_package(region,map_3d_package);
		} /* if (node_order_info=CREATE(FE */
		else
		{
			display_message(ERROR_MESSAGE,"interpolation_function_to_node_group "
				"CREATE(FE_node_order_info) failed ");
		}
		MANAGER_END_CACHE(FE_node)(node_manager);
		if (interpolation_node_group)
		{
			MANAGED_GROUP_END_CACHE(FE_node)(interpolation_node_group);
		}
		if (template_node)
		{
			DESTROY(FE_node)(&template_node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"interpolation_function_to_node_group."
			" Invalid arguments");
		interpolation_node_group =(struct GROUP(FE_node) *)NULL;
	}
	*the_node_order_info = node_order_info;
	LEAVE;

	return (interpolation_node_group);
}/*interpolation_function_to_node_group */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct GROUP(FE_element) *make_fit_elements(char *group_name,
	struct FE_node_order_info *node_order_info,
	struct FE_field_order_info *field_order_info,
	struct MANAGER(GROUP(FE_element))	*element_group_manager,
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager,enum Region_type region_type,
	int number_of_rows,int number_of_columns,FE_value *x_mesh,FE_value *y_mesh,
	struct Region *region)
/*******************************************************************************
LAST MODIFIED : 1 February 2001

DESCRIPTION :
Make the elements for the 3d map surface.
==============================================================================*/
{
	int addition,count,base_scale_factor_offset,dest_index,i,index,j,k,l,m,
		mesh_col,mesh_row,number_of_fields,number_of_grid_nodes,number_of_nodes,
		number_of_scale_factor_sets,*numbers_in_scale_factor_sets,
		return_code,source_index,number_of_components,number_of_derivatives,
		*temp_numbers_in_scale_factor_sets;
	FE_element_field_component_modify modify;
	FE_value height,width;
	struct CM_element_information element_identifier;
	struct FE_basis *basis,*linear_lagrange_basis,*cubic_hermite_basis;
	struct FE_element *element,*template_element;
	struct GROUP(FE_element) *element_group;
	struct FE_element_shape *shape;
	struct FE_node **element_nodes;
	struct FE_field *field;
	struct FE_element_field_component **components;
	struct Map_3d_package *map_3d_package=(struct Map_3d_package *)NULL;
	struct Standard_node_to_element_map **standard_node_map;
	void **scale_factor_set_identifiers,**temp_scale_factor_set_identifiers;
	int linear_lagrange_basis_type[4]=
	{
		2,LINEAR_LAGRANGE,0,LINEAR_LAGRANGE
	};
	int cubic_hermite_basis_type[4]=
	{
		2,CUBIC_HERMITE,0,CUBIC_HERMITE
	};
	int shape_type[3]=
	{
		LINE_SHAPE,0,0
	};

	ENTER(make_fit_elements);
	element_group=(struct GROUP(FE_element) *)NULL;
	if (basis_manager&&element_manager&&(map_3d_package=get_Region_map_3d_package(region)))
	{
		return_code=1;
		number_of_nodes=4;
		/* Arrange the nodes into an array,where the 1st number_of_columns+1 nodes
			are the first row, the 2nd the 2nd row, etc, for number_of_row+1 rows.
			I.e a grid stored in a 1D array. Each of the
			number_of_columns*number_of_row elements is formed by 4 nodes making up
			a square in this grid */
		number_of_grid_nodes=(number_of_rows+1)*(number_of_columns+1);
		if (ALLOCATE(element_nodes,struct FE_node *,number_of_grid_nodes))
		{
			switch (region_type)
			{
				case PATCH:
				{
					for(i=0;i<number_of_grid_nodes;i++)
					{
						element_nodes[i]=
							get_FE_node_order_info_node(node_order_info,i);
					}
				} break;
				case TORSO:
				{
					count=0;
					dest_index=0;
					for(i=0;i<number_of_rows+1;i++)
					{
						for(j=0;j<number_of_columns;j++)
						{
							source_index=count;
							element_nodes[dest_index]=get_FE_node_order_info_node(
								node_order_info,source_index);
							count++;
							dest_index++;
						}
						/* last columns same as the first*/
						source_index = count-number_of_columns;
						element_nodes[dest_index]=get_FE_node_order_info_node(
								node_order_info,source_index);
						dest_index++;
					}
				} break;
				case SOCK:
				{
					dest_index=0;
					source_index=0;
					/* first row all zero*/
					for(j=0;j<number_of_columns+1;j++)
					{
						element_nodes[dest_index]=get_FE_node_order_info_node(
							node_order_info,source_index);
						dest_index++;
					}
					count=1;
					for(i=1;i<(number_of_rows+1);i++)
					{
						for(j=0;j<number_of_columns;j++)
						{
							source_index=count;
							element_nodes[dest_index]=get_FE_node_order_info_node(
								node_order_info,source_index);
							count++;
							dest_index++;
						}
						/* last columns same as the first*/
						source_index = count-number_of_columns;
						element_nodes[dest_index]=get_FE_node_order_info_node(
							node_order_info,source_index);
						dest_index++;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"make_fit_elements.  Invalid region_type");
					return_code=0;
				} break;
			} /* switch (region_type) */
		}
		else
		{
			display_message(ERROR_MESSAGE,"make_fit_elements.  Not enough memory");
			return_code=0;
		}

		/* check the number of derivatives and components, as all nodes should
			have the same fields */
		number_of_fields =
			get_FE_field_order_info_number_of_fields(field_order_info);
		if (return_code)
		{
			for(i = 0; i < number_of_fields; i++)
			{
				field = get_FE_field_order_info_field(field_order_info, i);
				k=0;
				number_of_components=get_FE_field_number_of_components(field);
				while ((k<number_of_components)&&return_code)
				{
					number_of_derivatives=
						get_FE_node_field_component_number_of_derivatives(element_nodes[0],
						field,k);
					j=1;
					while ((j<number_of_grid_nodes)&&return_code)
					{
						if (number_of_derivatives!=
							get_FE_node_field_component_number_of_derivatives(
							element_nodes[j],field,k))
						{
							display_message(ERROR_MESSAGE,"make_fit_elements. "
								"number of derivatives, components mismatch");
							return_code=0;
						}
						j++;
					}
					k++;
				}
			}
		}
		/* make the bases */
		if (linear_lagrange_basis=
			make_FE_basis(linear_lagrange_basis_type,basis_manager))
		{
			ACCESS(FE_basis)(linear_lagrange_basis);
		}
		else
		{
			return_code=0;
		}
		if (cubic_hermite_basis=
			make_FE_basis(cubic_hermite_basis_type,basis_manager))
		{
			ACCESS(FE_basis)(cubic_hermite_basis);
		}
		else
		{
			return_code=0;
		}
		/* make the element shape */
		if (shape=CREATE(FE_element_shape)(2,shape_type))
		{
			ACCESS(FE_element_shape)(shape);
		}
		else
		{
			return_code=0;
		}
		if (!(element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
			(group_name,element_group_manager)))
		{
			display_message(ERROR_MESSAGE,
				"make_fit_elements.  Couldn't find element group");
			return_code=0;
		}
		/* create the scale_factor sets */
		number_of_scale_factor_sets=0;
		numbers_in_scale_factor_sets=(int *)NULL;
		scale_factor_set_identifiers=(void **)NULL;
		i=0;
		while (return_code&&(i<number_of_fields))
		{
			field=get_FE_field_order_info_field(field_order_info,i);
			number_of_components=get_FE_field_number_of_components(field);
			k=0;
			while (return_code&&(k<number_of_components))
			{
				number_of_derivatives=get_FE_node_field_component_number_of_derivatives(
						element_nodes[0],field,/*component_number*/k);
				/* check that field is bilinear or bicubic, set up scale factors
					accordingly */
				if ((number_of_derivatives==0)||(number_of_derivatives==3))
				{
					if (0==number_of_derivatives) /* bilinear */
					{
						addition =4;
						basis=linear_lagrange_basis;
					}
					else /* bicubic */
					{
						addition =16;
						basis=cubic_hermite_basis;
					}
					/* work out if scale_factor_set_identifiers/basis already used */
					j=0;
					while ((j<number_of_scale_factor_sets)&&
						(scale_factor_set_identifiers[j] != (void *)basis))
					{
						j++;
					}
					if (j==number_of_scale_factor_sets)
					{
						number_of_scale_factor_sets++;
						/* allocate and set scale factors to 1.0*/
						if (REALLOCATE(temp_scale_factor_set_identifiers,
							scale_factor_set_identifiers,void *,
							number_of_scale_factor_sets)&&
							REALLOCATE(temp_numbers_in_scale_factor_sets,
								numbers_in_scale_factor_sets,int,number_of_scale_factor_sets))
						{
							scale_factor_set_identifiers=temp_scale_factor_set_identifiers;
							numbers_in_scale_factor_sets=temp_numbers_in_scale_factor_sets;
							scale_factor_set_identifiers[number_of_scale_factor_sets-1]=
								(void *)basis;
							numbers_in_scale_factor_sets[number_of_scale_factor_sets-1]=
								addition;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"make_fit_elements.  Not enough memory for scale_factors");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"make_fit_elements.  Bilinear or bicubic bases only");
					return_code=0;
				}
				k++;
			}
			i++;
		}
		template_element=(struct FE_element *)NULL;
		if (return_code)
		{
			/* create the template element */
			element_identifier.type=CM_ELEMENT;
			element_identifier.number=1;
			if (template_element=CREATE(FE_element)(&element_identifier,
				(struct FE_element *)NULL))
			{
				ACCESS(FE_element)(template_element);
				/* set the element shape and node scale field info */
				if (set_FE_element_shape(template_element,shape)&&
					set_FE_element_node_scale_field_info(template_element,
					number_of_scale_factor_sets,scale_factor_set_identifiers,
						numbers_in_scale_factor_sets,number_of_nodes))
				{
					/* define the fields over the elements */
					i=0;
					while (return_code&&(i<number_of_fields))
					{
						field=get_FE_field_order_info_field(field_order_info,i);
						number_of_components=get_FE_field_number_of_components(field);
						if (ALLOCATE(components,struct FE_element_field_component *,
							number_of_components))
						{
							for (j=0;j<number_of_components;j++)
							{
								components[j]=(struct FE_element_field_component *)NULL;
							}
							j=0;
							while (return_code&&(j<number_of_components))
							{
								number_of_derivatives=
									get_FE_node_field_component_number_of_derivatives(
									element_nodes[0],field,/*component_number*/j);
								if (0==number_of_derivatives)
								{
									/* bilinear */
									addition=4;
									basis=linear_lagrange_basis;
								}
								else
								{
									/* bicubic */
									addition=16;
									basis=cubic_hermite_basis;
								}
								/* find the start of the scale factors for this basis */
								base_scale_factor_offset=0;
								k=0;
								while ((k<number_of_scale_factor_sets)&&
									((void *)basis != scale_factor_set_identifiers[k]))
								{
									base_scale_factor_offset += numbers_in_scale_factor_sets[k];
									k++;
								}
								if (strcmp("theta",get_FE_field_component_name(field,j)))
								{
									modify=(FE_element_field_component_modify)NULL;
								}
								else
								{
									modify=theta_increasing_in_xi1;
								}
								if (components[j]=CREATE(FE_element_field_component)(
									STANDARD_NODE_TO_ELEMENT_MAP,number_of_nodes,basis,modify))
								{
									/* create node map*/
									standard_node_map=(components[j])->
										map.standard_node_based.node_to_element_maps;
									m=0;
									for (k=0;k<number_of_nodes;k++)
									{
										if (*standard_node_map=CREATE(Standard_node_to_element_map)(
											/*node_index*/k,
											/*number_of_values*/1+number_of_derivatives))
										{
											for (l=0;l<(1+number_of_derivatives);l++)
											{
												(*standard_node_map)->nodal_value_indices[l]=l;
												(*standard_node_map)->scale_factor_indices[l]=
													base_scale_factor_offset+m;
												/* set scale_factors to 1 */
												template_element->information->scale_factors[
													(*standard_node_map)->scale_factor_indices[l]]=1.0;
												m++;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,"make_fit_elements.  "
												"Could not create Standard_node_to_element_map");
											return_code=0;
										}
										standard_node_map++;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"make_fit_elements.  Could not create component");
									return_code=0;
								}
								j++;
							}
							if (return_code)
							{
								if (!define_FE_field_at_element(template_element,
									field,components))
								{
									display_message(ERROR_MESSAGE,
										"make_fit_elements.  Could not define field at element");
									return_code=0;
								}
							}
							/* clean up the components */
							for (j=0;j<number_of_components;j++)
							{
								DESTROY(FE_element_field_component)(&(components[j]));
							}
							DEALLOCATE(components);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"make_fit_elements.  Could not allocate components");
							return_code=0;
						}
						i++;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"make_fit_elements.  Could not set element shape/field info");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"make_fit_elements.  Could not create template element");
				return_code=0;
			}
		}

		/* now have template_element. Make those elements! */
		if (return_code)
		{
			MANAGER_BEGIN_CACHE(FE_element)(element_manager);
			MANAGED_GROUP_BEGIN_CACHE(FE_element)(element_group);
			/* work through all elements */
			element_identifier.type=CM_ELEMENT;
			element_identifier.number=1;
			/* the coordinate field */
			field=get_FE_field_order_info_field(field_order_info,0);
			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					index = (i*(number_of_columns+1))+j;
					while (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
						&element_identifier,element_manager))
					{
						element_identifier.number++;
					}
					if (element=CREATE(FE_element)(&element_identifier,template_element))
					{
						if (set_FE_element_node(element,0,element_nodes[index])&&
							set_FE_element_node(element,1,element_nodes[index+1])&&
							set_FE_element_node(element,2,
								element_nodes[index+number_of_columns+1])&&
							set_FE_element_node(element,3,
								element_nodes[index+number_of_columns+2]))
						{
							/* set scale factors for fit field at element nodes*/
							/* the fit field (should really search by name in manager) */
							field=get_FE_field_order_info_field(field_order_info,1);
							/* see draw_2d_make_map for derivation of width,height*/
							mesh_row=number_of_rows-i;/*reversed so runs from max down to 1*/
							mesh_col=j+1;
							width=x_mesh[mesh_col]-x_mesh[mesh_col-1];
							height=y_mesh[mesh_row]-y_mesh[mesh_row-1];
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index],field,0/*component_number*/,
								FE_NODAL_VALUE,1.0);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index],field,0/*component_number*/,
								FE_NODAL_D_DS1,width);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index],field,0/*component_number*/,
								FE_NODAL_D_DS2,height);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index],field,0/*component_number*/,
								FE_NODAL_D2_DS1DS2,width*height);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+1],field,0/*component_number*/,
								FE_NODAL_VALUE,1.0);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+1],field,0/*component_number*/,
								FE_NODAL_D_DS1,width);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+1],field,0/*component_number*/,
								FE_NODAL_D_DS2,height);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+1],field,0/*component_number*/,
								FE_NODAL_D2_DS1DS2,width*height);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+number_of_columns+1],field,
								0/*component_number*/,FE_NODAL_VALUE,1.0);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+number_of_columns+1],field,
								0/*component_number*/,FE_NODAL_D_DS1,width);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+number_of_columns+1],field,
								0/*component_number*/,FE_NODAL_D_DS2,height);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+number_of_columns+1],field,
								0/*component_number*/,FE_NODAL_D2_DS1DS2,width*height);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+number_of_columns+2],field,
								0/*component_number*/,FE_NODAL_VALUE,1.0);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+number_of_columns+2],field,
								0/*component_number*/,FE_NODAL_D_DS1,width);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+number_of_columns+2],field,
								0/*component_number*/,FE_NODAL_D_DS2,height);
							FE_element_set_scale_factor_for_nodal_value(element,
								element_nodes[index+number_of_columns+2],field,
								0/*component_number*/,FE_NODAL_D2_DS1DS2,width*height);
							/* add element to manager*/
							if (ADD_OBJECT_TO_MANAGER(FE_element)(element,element_manager))
							{
								if (!ADD_OBJECT_TO_GROUP(FE_element)(element,element_group))
								{
									display_message(ERROR_MESSAGE,"make_fit_elements.  "
										"Could not add element to element_group");
									/* remove element from manager to destroy it */
									REMOVE_OBJECT_FROM_MANAGER(FE_element)(element,
										element_manager);
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"make_fit_elements.  "
									"Could not add element to element_manager");
								DESTROY(FE_element)(&element);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"make_fit_elements.  "
								"Could not set element nodes");
							DESTROY(FE_element)(&element);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"make_fit_elements.  "
							"Could not create element");
						return_code=0;
					}
				}
			}
			MANAGED_GROUP_END_CACHE(FE_element)(element_group);
			MANAGER_END_CACHE(FE_element)(element_manager);
		}
		if (return_code)
		{
			set_map_3d_package_element_group(map_3d_package,element_group);
		}
		else
		{
			element_group =(struct GROUP(FE_element) *)NULL;
		}

		/* clean up */
		if (template_element)
		{
			DEACCESS(FE_element)(&template_element);
		}
		if (numbers_in_scale_factor_sets)
		{
			DEALLOCATE(numbers_in_scale_factor_sets);
		}
		if (scale_factor_set_identifiers)
		{
			DEALLOCATE(scale_factor_set_identifiers);
		}
		if (shape)
		{
			DEACCESS(FE_element_shape)(&shape);
		}
		if (cubic_hermite_basis)
		{
			DEACCESS(FE_basis)(&cubic_hermite_basis);
		}
		if (linear_lagrange_basis)
		{
			DEACCESS(FE_basis)(&linear_lagrange_basis);
		}
		if (element_nodes)
		{
			DEALLOCATE(element_nodes);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_fit_elements."
			" Invalid arguments");
		element_group =(struct GROUP(FE_element) *)NULL;
	}
	LEAVE;

	return (element_group);
} /* make_fit_elements */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int change_fit_node_group_values(struct FE_node_order_info
	*node_order_info,struct FE_field *fit_field,
	struct Interpolation_function *function,char *name,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_element))	*element_group_manager,
	struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED :14 November 2000

DESCRIPTION :
Given the <function> and <package>  <node_order_info> and <name>,
Checks that a node group of name <name> exists, searches the node group for the
nodes in <node_order_info>, and fills them in with values from <function>.
*******************************************************************************/
{
	enum Region_type region_type;
	int count,f_index,i,j,number_of_columns,number_of_nodes,
		number_of_rows,return_code;
	FE_value dfdx,dfdy,d2fdxdy,f;
	struct GROUP(FE_node) *existing_node_group;
	struct FE_node *node_managed,*node;

	ENTER(change_fit_node_group_value);
	return_code=1;
	if (node_order_info&&function&&name&&node_group_manager&&element_group_manager)
	{
		region_type=function->region_type;
		number_of_rows=function->number_of_rows;
		number_of_columns=function->number_of_columns;
		number_of_nodes=node_order_info->number_of_nodes;

		if ((existing_node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
			name,node_group_manager))&&(FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
				(name,element_group_manager)))
		{
			MANAGER_BEGIN_CACHE(FE_node)(node_manager);
			switch (region_type)
			{
				case SOCK:
				{
					f=function->f[0];
					dfdx=function->dfdx[0];
					dfdy=function->dfdy[0];
					d2fdxdy=function->d2fdxdy[0];
					/* set node values here. */
					/* get the apex node from the node_order_info, check  it's in the group */
					node_managed=get_FE_node_order_info_node(node_order_info,0);
					if (node_managed=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
						get_FE_node_cm_node_identifier(node_managed),existing_node_group))
					{
						/* create a node to work with */
						node=CREATE(FE_node)(0,(struct FE_node *)NULL);
						/* copy it from the manager */
						if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
							(node,node_managed))
						{
							set_mapping_FE_node_fit_values(node,fit_field,0/*component_number*/,
								f,dfdx,dfdy,d2fdxdy);
							/* copy it back into the manager */
							MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
								(node_managed,node,node_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"change_fit_node_group_values. MANAGER_COPY_WITH_IDENTIFIER failed ");
							return_code=0;
						}
						/* destroy the working copy */
						DESTROY(FE_node)(&node);
					}
					else
					{
						display_message(ERROR_MESSAGE,
								"change_fit_node_group_values. Can't find node in group ");
							return_code=0;
					}
					count=1;
					while((count<number_of_nodes)&&return_code)
					{
						/* done apex, so skip first row */
						for(i=1;i<number_of_rows+1;i++)
						{
							/* skip last column, as same as first */
							for(j=0;j<number_of_columns;j++)
							{
								f_index=j+(i*(number_of_columns+1));
								f=function->f[f_index];
								dfdx=function->dfdx[f_index];
								dfdy=function->dfdy[f_index];
								d2fdxdy=function->d2fdxdy[f_index];
								/* set node values here.  */
								node_managed=get_FE_node_order_info_node(node_order_info,count);
								if (node_managed=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
									get_FE_node_cm_node_identifier(node_managed),existing_node_group))
								{
									/* create a node to work with */
									node=CREATE(FE_node)(0,(struct FE_node *)NULL);
									/*copy it from the manager */
									if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
										(node,node_managed))
									{
										set_mapping_FE_node_fit_values(node,fit_field,0/*component_number*/,
											f,dfdx,dfdy,d2fdxdy);
										/* copy it back into the manager */
										MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
											(node_managed,node,node_manager);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"change_fit_node_group_values. MANAGER_COPY_WITH_IDENTIFIER failed ");
										return_code=0;
									}
									/* destroy the working copy */
									DESTROY(FE_node)(&node);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"change_fit_node_group_values. Can't find node in group");
									return_code=0;
								}
								count++;
							} /* for(j= */
						} /* for(i= */
					} /* while(count */
				} break;
				case TORSO:
				{
					count=0;
					while((count<number_of_nodes)&&return_code)
					{
						/* do all rows */
						for(i=0;i<number_of_rows+1;i++)
						{
							/* skip last column, as same as first*/
							for(j=0;j<number_of_columns;j++)
							{
								f_index=j+(i*(number_of_columns+1));
								f=function->f[f_index];
								dfdx=function->dfdx[f_index];
								dfdy=function->dfdy[f_index];
								d2fdxdy=function->d2fdxdy[f_index];
								/* set node values here.  */
								node_managed=get_FE_node_order_info_node(node_order_info,count);
								if (node_managed=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
									get_FE_node_cm_node_identifier(node_managed),existing_node_group))
								{
									/* create a node to work with */
									node=CREATE(FE_node)(0,(struct FE_node *)NULL);
									/*copy it from the manager */
									if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
										(node,node_managed))
									{
										set_mapping_FE_node_fit_values(node,fit_field,0/*component_number*/,
											f,dfdx,dfdy,d2fdxdy);
										/* copy it back into the manager */
										MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
											(node_managed,node,node_manager);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"change_fit_node_group_values. MANAGER_COPY_WITH_IDENTIFIER failed ");
										return_code=0;
									}
									/* destroy the working copy */
									DESTROY(FE_node)(&node);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"change_fit_node_group_values. Can't find node in group");
									return_code=0;
								}
								count++;
							} /* for(j= */
						} /* for(i= */
					} /* while(count */
				} break;
				case PATCH:
				{
					count=0;
					while((count<number_of_nodes)&&return_code)
					{
						/* do all rows */
						for(i=0;i<number_of_rows+1;i++)
						{
							/* do all columns */
							for(j=0;j<number_of_columns+1;j++)
							{
								f=function->f[count];
								dfdx=function->dfdx[count];
								dfdy=function->dfdy[count];
								d2fdxdy=function->d2fdxdy[count];
								/* set node values here.  */
								node_managed=get_FE_node_order_info_node(node_order_info,count);
								if (node_managed=FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
									get_FE_node_cm_node_identifier(node_managed),existing_node_group))
								{
									/* create a node to work with */
									node=CREATE(FE_node)(0,(struct FE_node *)NULL);
									/*copy it from the manager */
									if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
										(node,node_managed))
									{
										set_mapping_FE_node_fit_values(node,fit_field,0/*component_number*/,
											f,dfdx,dfdy,d2fdxdy);
										/* copy it back into the manager */
										MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
											(node_managed,node,node_manager);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"change_fit_node_group_values MANAGER_COPY_WITH_IDENTIFIER failed ");
										return_code=0;
									}
									/* destroy the working copy */
									DESTROY(FE_node)(&node);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"change_fit_node_group_values. Can't find node in group");
									return_code=0;
								}
								count++;
							} /* for(j= */
						} /* for(i= */
					} /* while(count */
				} break; /* case PATCH:*/
			} /* switch (region_type) */
			MANAGER_END_CACHE(FE_node)(node_manager);
		}
		else
		{
			display_message(ERROR_MESSAGE,"change_fit_node_group_values."
			" can't find node/elemet groups");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"change_fit_node_group_values."
			" Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* change_fit_node_group_values */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int torso_node_is_in_z_range(struct FE_node *torso_node,void *z_value_data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Conditional function returning true if <node>  in the z range of <data_void>.
==============================================================================*/
{
	FE_value torso_node_z;
	int return_code;
	struct FE_field_component component;
	struct Z_value_data *z_value_data;

	ENTER(torso_node_is_in_z_range);
	return_code=0;
	if (torso_node&&(z_value_data=(struct Z_value_data *)z_value_data_void))
	{
		component.field=z_value_data->torso_position_field;
		component.number=2;
		get_FE_nodal_FE_value_value(torso_node,&component,0,FE_NODAL_VALUE,
			/*time*/0,&torso_node_z);
		if ((torso_node_z>=z_value_data->map_min_z)&&
			(torso_node_z<=z_value_data->map_max_z))
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"torso_node_is_in_z_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* torso_node_is_in_z_range  */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int add_torso_elements_if_nodes_in_z_range(
	struct FE_element *torso_element,void *element_torso_data_void)
/*******************************************************************************
LAST MODIFIED : 30 July 2001

DESCRIPTION :
Get list of nodes in <torso_element>.
If <torso_element> is a CM_ELEMENT  AND has 4 nodes and all nodes in
<torso_element> are also in element_torso_data->torso_nodes_in_z_range,
adds the element to element_torso_data->mapped_torso_element_group.
Also sets the fit field at the torso nodes
==============================================================================*/
{
	int i,number_of_nodes,return_code;
	struct Element_torso_data *element_torso_data;
	struct FE_node **nodes_in_element;

	ENTER(add_torso_elements_if_nodes_in_z_range);
	return_code=1;
	element_torso_data=(struct Element_torso_data *)NULL;
	if (torso_element&&element_torso_data_void&&(element_torso_data
		=(struct Element_torso_data *)element_torso_data_void))
	{
		/* only deal with elements, not lines */
		if (CM_ELEMENT==torso_element->cm.type)
		{
			/* get all nodes in element */
			if (calculate_FE_element_field_nodes(torso_element,
				(struct FE_field *)NULL,&number_of_nodes,&nodes_in_element))
			{
				/* do nothing if not 4 nodes */
				if (4==number_of_nodes)
				{
					/* determine if all element nodes in z range list */
					i=0;
					while ((i<number_of_nodes)&&IS_OBJECT_IN_LIST(FE_node)(
						nodes_in_element[i],element_torso_data->torso_nodes_in_z_range))
					{
						i++;
					}
					if (i>=number_of_nodes)
					{
						/* add torso element to group*/
						ADD_OBJECT_TO_GROUP(FE_element)(torso_element,
							element_torso_data->mapped_torso_element_group);
					}
				}
				for (i=0;i<number_of_nodes;i++)
				{
					DEACCESS(FE_node)(nodes_in_element+i);
				}
				DEALLOCATE(nodes_in_element);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_torso_elements_if_nodes_in_z_range. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* add_torso_elements_if_nodes_in_z_range */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int make_mapped_torso_node_and_element_groups(struct Region *region,
	struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
Makes the node and element groups for the mapped torso.
Does by adding all the nodes/elements from the loaded default torso that have
their z position within the z range of the node_group nodes. node_group
nodes z values are defined by the electrodes.
==============================================================================*/
{
	int return_code;
	char *default_torso_group_name;
	char mapped_torso_group_name[]="mapped_torso";
	FE_value min_x,max_x,min_y,max_y,min_z,max_z;
	struct Element_torso_data element_torso_data;
	struct FE_field *electrode_position_field,*torso_position_field;
	struct FE_node *torso_node;
	struct FE_node_list_conditional_data list_conditional_data;
	struct GROUP(FE_element) *default_torso_element_group,
		*mapped_torso_element_group;
	struct GROUP(FE_node) *node_group,*mapped_torso_node_group,
		*default_torso_node_group;
	struct LIST(FE_node) *torso_correct_z_node_list;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_node) *node_manager;

	struct Map_3d_package *map_3d_package;
	struct Z_value_data z_value_data;

	ENTER(make_mapped_torso_node_and_element_groups);
	element_manager=(struct MANAGER(FE_element) *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	node_group=(struct GROUP(FE_node) *)NULL;
	default_torso_node_group=(struct GROUP(FE_node) *)NULL;
	electrode_position_field=(struct FE_field *)NULL;
	torso_position_field=(struct FE_field *)NULL;
	torso_node=(struct FE_node *)NULL;
	mapped_torso_element_group=(struct GROUP(FE_element) *)NULL;
	default_torso_element_group=(struct GROUP(FE_element) *)NULL;
	mapped_torso_node_group=(struct GROUP(FE_node) *)NULL;
	torso_correct_z_node_list=(struct LIST(FE_node) *)NULL;
	if (region&&(default_torso_group_name=
		get_unemap_package_torso_group_name(unemap_package))
		&&(element_group_manager=get_unemap_package_element_group_manager(unemap_package))
		&&(node_group_manager=get_unemap_package_node_group_manager(unemap_package))
		&&(node_manager=get_unemap_package_node_manager(unemap_package))
		&&(element_manager=get_unemap_package_element_manager(unemap_package))
		&&(data_group_manager=get_unemap_package_data_group_manager(unemap_package))
		&&(default_torso_element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),
		name)(default_torso_group_name,element_group_manager)))
	{
		return_code=1;
		map_3d_package=get_Region_map_3d_package(region);
		/* get min and max of cylinder */
		node_group=get_Region_unrejected_node_group(region);
		electrode_position_field=get_Region_electrode_position_field(region);
		get_node_group_position_min_max(node_group,electrode_position_field,
			&min_x,&max_x,&min_y,&max_y,&min_z,&max_z);
		/* now have min,max z to work with */
		default_torso_node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),
			name)(default_torso_group_name,node_group_manager);
		torso_node=FIRST_OBJECT_IN_GROUP_THAT(FE_node)
			((GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL,default_torso_node_group);
		torso_position_field=get_FE_node_default_coordinate_field(torso_node);
		/* compile a list of torso nodes with the right z range */
		z_value_data.map_min_z=min_z;
		z_value_data.map_max_z=max_z;
		z_value_data.torso_position_field=torso_position_field;
		torso_correct_z_node_list=CREATE_LIST(FE_node)();
		list_conditional_data.node_list=torso_correct_z_node_list;
		list_conditional_data.function=torso_node_is_in_z_range;
		list_conditional_data.user_data=(void *)(&z_value_data);
		FOR_EACH_OBJECT_IN_GROUP(FE_node)(ensure_FE_node_is_in_list_conditional,
			(void *)&list_conditional_data,default_torso_node_group);
		/* create same name node,element,data groups so can do graphics */
		mapped_torso_node_group=make_node_and_element_and_data_groups(
			node_group_manager,node_manager,element_manager,element_group_manager,
			data_group_manager,mapped_torso_group_name);
		MANAGED_GROUP_BEGIN_CACHE(FE_node)(mapped_torso_node_group);
		mapped_torso_element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
			(mapped_torso_group_name,element_group_manager);
		MANAGED_GROUP_BEGIN_CACHE(FE_element)(mapped_torso_element_group);
		set_map_3d_package_electrodes_max_z(map_3d_package,max_z);
		set_map_3d_package_electrodes_min_z(map_3d_package,min_z);
		element_torso_data.torso_nodes_in_z_range=torso_correct_z_node_list;
		element_torso_data.mapped_torso_element_group=mapped_torso_element_group;
		/* add torso elements to group if all nodes have are in z range */
		FOR_EACH_OBJECT_IN_GROUP(FE_element)(add_torso_elements_if_nodes_in_z_range,
			(void *)&element_torso_data,default_torso_element_group);
		/* did we get any matches?*/
		if (0==NUMBER_IN_GROUP(FE_element)(mapped_torso_element_group))
		{
			/* this is more of a warning than an error, I've made it an ERROR_MESSAGE */
			/* to ensure the user notices it! (warning messages go to the console) */
			display_message(ERROR_MESSAGE,
				"None of the rig's torso electrodes match the Default Torso");
		}
		/* no longer needed, so destroy */
		DESTROY_LIST(FE_node)(&torso_correct_z_node_list);
		/* to fill node group */
		FOR_EACH_OBJECT_IN_GROUP(FE_element)
			(ensure_top_level_FE_element_nodes_are_in_group,
				(void *)mapped_torso_node_group,mapped_torso_element_group);
		set_map_3d_package_mapped_torso_node_group(map_3d_package,
			mapped_torso_node_group);
		set_map_3d_package_mapped_torso_element_group(map_3d_package,
			mapped_torso_element_group);
		MANAGED_GROUP_END_CACHE(FE_node)(mapped_torso_node_group);
		MANAGED_GROUP_END_CACHE(FE_element)(mapped_torso_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_mapped_torso_node_and_element_groups. "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* make_mapped_torso_node_and_element_groups */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int set_map_fit_field_at_torso_node(struct FE_node *torso_node,
	struct FE_element *torso_element,struct Map_3d_package *map_3d_package,
	int first_torso_element_number)
/*******************************************************************************
LAST MODIFIED : 16 October 2000

DESCRIPTION :
Defines and sets the value of the fit field at <torso_node>.
===============================================================================*/
{
	FE_value cylinder_jacobian[2],cylinder_xi_coords[2],df_ds1,df_ds2,d2f_ds1ds2,
		df_du1,df_du2,df_dxi1,df_dxi2,d2f_dxi1dxi2,dx_dxi1,dx_dxi2,dy_dxi1,dy_dxi2,
		dz_dxi1,dz_dxi2,fit,map_theta,map_z,minusY_div_X2plusY2,Ntheta_div_2PI,
		Ntheta_plus_xi,Nz_div_Zrange,Nz_plus_xi,scale_factor,torso_jacobian[6],
		torso_xi_coords[2],torso_node_x,torso_node_y,torso_node_z,torso_pos[3],
		X2plusY2,	X_div_X2plusY2,z_range;
	FE_value map_min_z,map_max_z,num_z_elements,num_theta_elements;

	int element_number,Nz,Ntheta,return_code;
	struct CM_element_information element_information;
	struct FE_element *cylinder_element;
	struct FE_element_field_values element_field_values;
	struct FE_field_component component;
	struct FE_field *map_fit_field,*torso_position_field;
	struct FE_node *node;
	struct MANAGER(FE_node) *node_manager;

	ENTER(set_map_fit_field_at_torso_node);
	return_code=1;
	cylinder_element=(struct FE_element *)NULL;
	map_fit_field=(struct FE_field *)NULL;
	torso_position_field=(struct FE_field *)NULL;
	node=(struct FE_node *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	if (torso_node&&(torso_position_field=
		get_FE_node_default_coordinate_field(torso_node))&&map_3d_package&&torso_element&&
		(node_manager=map_3d_package->node_manager))
	{
		map_min_z=get_map_3d_package_electrodes_min_z(map_3d_package);
		map_max_z=get_map_3d_package_electrodes_max_z(map_3d_package);
		num_z_elements=get_map_3d_package_number_of_map_rows(map_3d_package);
		num_theta_elements=get_map_3d_package_number_of_map_columns(map_3d_package);
		map_fit_field=get_map_3d_package_fit_field(map_3d_package);
		/* field should already defined at node */
		if (FE_field_is_defined_at_node(map_fit_field,torso_node))
		{
			component.field=torso_position_field;
			component.number=0;
			get_FE_nodal_FE_value_value(torso_node,&component,0,FE_NODAL_VALUE,
				/*time*/0,&torso_node_x);
			component.number=1;
			get_FE_nodal_FE_value_value(torso_node,&component,0,FE_NODAL_VALUE,
				/*time*/0,&torso_node_y);
			component.number=2;
			get_FE_nodal_FE_value_value(torso_node,&component,0,FE_NODAL_VALUE,
				/*time*/0,&torso_node_z);
			/*now have torso x,y,z determine map theta,z (don't need r)*/
			/* add PI as want map_theta in range 0->2PI,atan2 returns -PI->PI */
			map_theta=atan2(torso_node_y,torso_node_x) +PI;
			/*map r is arbitary and not used*/
			map_z=torso_node_z;
			/* now determine element number and Xi values */
			Ntheta_plus_xi=num_theta_elements*(map_theta/(2*PI));
			/* Ntheta is the theta element number, i.e the column number */
			Ntheta=(int)(Ntheta_plus_xi);
			/* handle case with theta point exactly on 2PI*/
			if (Ntheta>=num_theta_elements)
			{
				Ntheta=num_theta_elements-1;
			}
			/* cylinder_xi_coords[0] is Xi in the theta direction*/
			cylinder_xi_coords[0]=Ntheta_plus_xi-Ntheta;
			z_range=map_max_z-map_min_z;
			Nz_plus_xi=num_z_elements*((map_z-map_min_z)/(z_range));
			/* Nz is the z element number, i.e the row number */
			Nz=(int)(Nz_plus_xi);
			/* handle case with z point exactly on cylinder edge*/
			if (Nz>=num_z_elements)
			{
				Nz=num_z_elements-1;
			}
			/* cylinder_xi_coords[1] is Xi in the z direction*/
			cylinder_xi_coords[1]=Nz_plus_xi-Nz;
			element_number=Nz*num_theta_elements+Ntheta+1;
			/* get map element, and value at element */
			element_information.type=CM_ELEMENT;
			element_information.number=element_number+first_torso_element_number-1;
			cylinder_element=FIND_BY_IDENTIFIER_IN_GROUP(FE_element,identifier)
				(&element_information,map_3d_package->element_group);
			if (cylinder_element)
			{
				/* get the cylinder fit value and jacobian */
				if (calculate_FE_element_field_values(cylinder_element,map_fit_field,
					/*time*/0,/*calculate_derivatives */1,&element_field_values,
					/*top_level_element*/(struct FE_element *)NULL))
				{
					/* calculate the value for the element field */
					return_code=calculate_FE_element_field(0,/*component_number*/
						&element_field_values,cylinder_xi_coords,&fit,cylinder_jacobian);
					df_du1=cylinder_jacobian[0];
					df_du2=cylinder_jacobian[1];
					clear_FE_element_field_values(&element_field_values);
				}
				/* get the torso position and jacobian */
				torso_xi_coords[0]=0;
				torso_xi_coords[1]=0;
				if (calculate_FE_element_field_values(torso_element,
					torso_position_field,/*time*/0,/*calculate_derivatives */1,
					&element_field_values,/*top_level_element*/(struct FE_element *)NULL))
				{
					/* calculate the value for the element field */
					return_code=calculate_FE_element_field(/*component_number*/-1,
					&element_field_values,torso_xi_coords,torso_pos,torso_jacobian);
					dx_dxi1=torso_jacobian[0];
					dx_dxi2=torso_jacobian[1];
					dy_dxi1=torso_jacobian[2];
					dy_dxi2=torso_jacobian[3];
					dz_dxi1=torso_jacobian[4];
					dz_dxi2=torso_jacobian[5];
					clear_FE_element_field_values(&element_field_values);
				}
				X2plusY2=(torso_node_x*torso_node_x)+(torso_node_y*torso_node_y);
				minusY_div_X2plusY2=-torso_node_y/X2plusY2;
				X_div_X2plusY2=torso_node_x/X2plusY2;
				Ntheta_div_2PI=Ntheta/(2*PI);
				Nz_div_Zrange=Nz/z_range;
				df_dxi1=df_du1*(minusY_div_X2plusY2*Ntheta_div_2PI*dx_dxi1 +
					X_div_X2plusY2*Ntheta_div_2PI*dy_dxi1) +df_du2*(Nz_div_Zrange*dz_dxi1);
				df_dxi2=df_du2*(minusY_div_X2plusY2*Ntheta_div_2PI*dx_dxi2 +
					X_div_X2plusY2*Ntheta_div_2PI*dy_dxi2) +df_du2*(Nz_div_Zrange*dz_dxi2);
				d2f_dxi1dxi2=df_dxi1*df_dxi2;
#if defined (DEBUG)
				printf(" Component %d \n",0);
#endif /* defined (DEBUG)*/
				FE_element_get_scale_factor_for_nodal_value(torso_element,torso_node,
					torso_position_field,0/*component_number*/,FE_NODAL_VALUE,&scale_factor);
#if defined (DEBUG)
				printf("   FE_NODAL_VALUE scale factor  = %f ",scale_factor);
				printf(" fit = %f ",fit);
#endif /* defined (DEBUG)*/
				fit=fit/scale_factor;
#if defined (DEBUG)
				printf(" fit/scale factor = %f \n",fit);
#endif /* defined (DEBUG)*/
				FE_element_get_scale_factor_for_nodal_value(torso_element,torso_node,
					torso_position_field,0/*component_number*/,FE_NODAL_D_DS1,&scale_factor);
				df_ds1=df_dxi1/scale_factor;
#if defined (DEBUG)
				printf("   FE_NODAL_D_DS1 scale factor  = %f ",scale_factor);
				printf(" df_dxi1 = %f ",df_dxi1);
				printf(" df_ds1 = %f \n",df_ds1);
#endif /* defined (DEBUG)*/
				FE_element_get_scale_factor_for_nodal_value(torso_element,torso_node,
					torso_position_field,0/*component_number*/,FE_NODAL_D_DS2,&scale_factor);
				df_ds2=df_dxi2/scale_factor;
#if defined (DEBUG)
				printf("   FE_NODAL_D_DS2 scale factor  = %f ",scale_factor);
				printf(" df_dxi1 = %f ",df_dxi1);
				printf(" df_ds1 = %f \n",df_ds1);
#endif /* defined (DEBUG)*/
				FE_element_get_scale_factor_for_nodal_value(torso_element,torso_node,
					torso_position_field,0/*component_number*/,FE_NODAL_D2_DS1DS2,
					&scale_factor);
				d2f_ds1ds2=d2f_dxi1dxi2/scale_factor;
#if defined (DEBUG)
				printf("   FE_NODAL_D2_DS1DS2 scale factor  = %f ",scale_factor);
				printf(" d2f_dxi1dxi2 = %f ",d2f_dxi1dxi2);
				printf(" d2f_ds1ds2 = %f \n",d2f_ds1ds2);
#endif /* defined (DEBUG)*/
				/* and set the nodal values  */

				/* create a node to work with */
				node=CREATE(FE_node)(0,(struct FE_node *)NULL);
				/* copy it from the manager */
				if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
					(node,torso_node))
				{
					set_mapping_FE_node_fit_values(node,map_fit_field,0/*component_number*/,
					fit,df_ds1,df_ds2,d2f_ds1ds2);
					/* copy it back into the manager */
					MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
						(torso_node,node,node_manager);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_map_fit_field_at_torso_node. MANAGER_COPY_WITH_IDENTIFIER failed ");
					return_code=0;
				}
				/* destroy the working copy */
				DESTROY(FE_node)(&node);
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_map_fit_field_at_torso_node."
					" can't find element");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_map_fit_field_at_torso_node."
				" field undefined at node");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_fit_field_at_torso_node. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_fit_field_at_torso_node */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int set_map_fit_field_if_required(struct FE_node *torso_node,
	void *map_value_torso_data_void)
/*******************************************************************************
LAST MODIFIED : 9 November 2000

DESCRIPTION :
Set the  nodal values of the fit field at the <torso_node>, if haven't already
done so. Maintains a list of processed nodes, so can tell which nodes have
already set.
==============================================================================*/
{
	int return_code;
	struct Map_value_torso_data *map_value_torso_data;

	ENTER(set_map_fit_field_if_required);
	map_value_torso_data=(struct Map_value_torso_data *)NULL;
	if (torso_node&&map_value_torso_data_void&&
		(map_value_torso_data=(struct Map_value_torso_data *)map_value_torso_data_void))
	{
		/* if object is already in processed list, have nothing to do*/
		if (!(IS_OBJECT_IN_LIST(FE_node)(torso_node,map_value_torso_data->processed_nodes)))
		{
			/* set the fit field at the node */
			if (return_code=set_map_fit_field_at_torso_node(torso_node,
				map_value_torso_data->torso_element,map_value_torso_data->map_3d_package,
				map_value_torso_data->first_torso_element_number))
			{
				/* add to the list of processed nodes */
				return_code=ADD_OBJECT_TO_LIST(FE_node)
					(torso_node,map_value_torso_data->processed_nodes);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_fit_field_if_required. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_fit_field_if_required */

static int set_map_fit_field_at_torso_element_nodes(
	struct FE_element *torso_element,void *map_value_torso_data_void)
/*******************************************************************************
LAST MODIFIED : 30 July 2001

DESCRIPTION :
Set the  nodal values of the fit field at the <torso_element> nodes
==============================================================================*/
{
	int i,number_of_nodes,return_code;
	struct FE_node **nodes_in_element;
	struct Map_value_torso_data *map_value_torso_data;

	ENTER(set_map_fit_field_at_torso_element_nodes);
	return_code=0;
	map_value_torso_data=(struct Map_value_torso_data *)NULL;
	if (torso_element&&(map_value_torso_data=
		(struct Map_value_torso_data *)map_value_torso_data_void))
	{
		/* get all nodes in element */
		if (calculate_FE_element_field_nodes(torso_element,
			(struct FE_field *)NULL,&number_of_nodes,&nodes_in_element))
		{
			/* for each node, set fit field value, if haven't done so already */
			map_value_torso_data->torso_element=torso_element;
			for (i=0;i<number_of_nodes;i++)
			{
				set_map_fit_field_if_required(nodes_in_element[i],
					(void *)map_value_torso_data);
			}
			for (i=0;i<number_of_nodes;i++)
			{
				DEACCESS(FE_node)(nodes_in_element+i);
			}
			DEALLOCATE(nodes_in_element);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_fit_field_at_torso_element_nodes. "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_fit_field_at_torso_element_nodes */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int set_torso_fit_field_values(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
set the nodal values of the fit field at the nodes of the elements in
<map_3d_package->mapped_torso_element_group>
==============================================================================*/
{
	int first_element_number,return_code;
	struct CM_element_information element_information;
	struct FE_element *first_element;
	struct GROUP(FE_element) *mapped_torso_element_group,*element_group;
	struct Map_value_torso_data map_value_torso_data;

	ENTER(set_torso_fit_field_values);
	mapped_torso_element_group=(struct GROUP(FE_element) *)NULL;
	element_group=(struct GROUP(FE_element) *)NULL;
	first_element=(struct FE_element *)NULL;
	if (map_3d_package)
	{
		return_code=1;
		/* find the identifier of the first element */
		element_group=get_map_3d_package_element_group(map_3d_package);
		first_element=FIRST_OBJECT_IN_GROUP_THAT(FE_element)(FE_element_is_top_level,
				NULL,element_group);
		element_information=first_element->cm;
		first_element_number=element_information.number;

		mapped_torso_element_group=
			get_map_3d_package_mapped_torso_element_group(map_3d_package);
		/*  set the element's nodal values  */
		map_value_torso_data.map_3d_package=map_3d_package;
		map_value_torso_data.first_torso_element_number=first_element_number;
		/* torso_element set up later, in set_map_fit_field_at_torso_element_nodes */
		map_value_torso_data.torso_element=(struct FE_element *)NULL;
		map_value_torso_data.processed_nodes=CREATE_LIST(FE_node)();
		MANAGER_BEGIN_CACHE(FE_node)(map_3d_package->node_manager);
		FOR_EACH_OBJECT_IN_GROUP(FE_element)(set_map_fit_field_at_torso_element_nodes,
			(void *)&map_value_torso_data,mapped_torso_element_group);
		MANAGER_END_CACHE(FE_node)(map_3d_package->node_manager);
		DESTROY_LIST(FE_node)(&map_value_torso_data.processed_nodes);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_torso_fit_field_values. "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_torso_fit_field_values*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static char *make_fit_name(struct Region *region, char *name)
/*******************************************************************************
LAST MODIFIED :1 March 2001

DESCRIPTION :
Constructs the fit name from the region.
*******************************************************************************/
{
	char *fit_name,*fit_str = "_fit",*patch_str="_patch",region_num_string[10],
		*sock_str="_sock",
		*torso_str="_torso",*type_str;
	int string_error,string_length;

	ENTER(make_fit_name);
	fit_name=(char *)NULL;
	type_str=(char *)NULL;
	string_error=0;
	if (name&&region)
	{
		/* construct the fit node group's name, from the name and region type*/
		string_length = strlen(name);
		string_length += strlen(fit_str);
		string_length++;
		switch (region->type)
		{
			case PATCH:
			{
				type_str=patch_str;
			} break;
			case SOCK:
			{
				type_str=sock_str;
			} break;
			case TORSO:
			{
				type_str=torso_str;
			} break;
		}
		string_length += strlen(type_str);
		string_length++;
		if (ALLOCATE(fit_name,char,string_length))
		{
			strcpy(fit_name,name);
			strcat(fit_name,fit_str);
			strcat(fit_name,type_str);
			/*append the region number to the name, to ensure it's unique*/
			sprintf(region_num_string,"%d",region->number);
			append_string(&fit_name,region_num_string,&string_error);
		}
		else
		{
			display_message(ERROR_MESSAGE,"make_fit_name. Out of memory");
			fit_name=(char *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_fit_name. Invalid argument(s)");
	}
	LEAVE;
	
	return (fit_name);
} /* make_fit_name */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int make_fit_node_and_element_groups(
	struct FE_field_order_info *field_order_info,
	struct Interpolation_function *function,struct Unemap_package *package,
	char *name,FE_value sock_lambda,FE_value torso_major_r,
	FE_value torso_minor_r,FE_value patch_z,struct Region *region,
	int nodes_rejected_or_accepted)
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Given the <function> and <package>, determines if  identical node and element
groups will already exist for the values in <function> and <package>.
If the identical groups already exist, simply change the values stored at the nodes, with
change_fit_node_group_values.
If the identical groups don't exist, create the groups and fill them in, with
interpolation_function_to_node_group and make_fit_elements
If <region>->type is TORSO, and we have a default torso loaded,
do similar with the mapped torso node group
*******************************************************************************/
{
	char *fit_name;
	int return_code;
	struct FE_node_order_info *node_order_info;
	struct Map_3d_package *map_3d_package;
	struct GROUP(FE_element) *element_group,*mapped_torso_element_group,
		*delaunay_torso_element_group;

	ENTER(make_fit_node_and_element_groups);
	fit_name=(char *)NULL;
	node_order_info = (struct FE_node_order_info *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	element_group=(struct GROUP(FE_element) *)NULL;
	mapped_torso_element_group=(struct GROUP(FE_element) *)NULL;
	delaunay_torso_element_group=(struct GROUP(FE_element) *)NULL;
	if (function&&package&&name)
	{
		return_code=1;
		if (fit_name=make_fit_name(region,name))
		{
			/* check if the function properties are the same as the last one, i.e */
			/* identical node and element groups should already exist, and that no */
			/* electrodes have recently been accpeted or rejected */
			map_3d_package=get_Region_map_3d_package(region);
			if (map_3d_package)
			{
				element_group=get_map_3d_package_element_group(map_3d_package);
				mapped_torso_element_group=
					get_map_3d_package_mapped_torso_element_group(map_3d_package);
				delaunay_torso_element_group=
					get_map_3d_package_delaunay_torso_element_group(map_3d_package);
			}
			if (map_3d_package&&(get_map_3d_package_number_of_map_rows(map_3d_package)
					==function->number_of_rows)&&
				(get_map_3d_package_number_of_map_columns(map_3d_package)
					==function->number_of_columns)&&(region->type
					==function->region_type)&&(!nodes_rejected_or_accepted)&&
				(element_group||mapped_torso_element_group||delaunay_torso_element_group)&&
				(!strcmp(get_map_3d_package_fit_name(map_3d_package),fit_name)))
			{
				/* just have to alter the nodal values */
				change_fit_node_group_values(get_map_3d_package_node_order_info(map_3d_package),
					get_map_3d_package_fit_field(map_3d_package),function,fit_name,
					get_unemap_package_node_group_manager(package),
					get_unemap_package_element_group_manager(package),
					get_unemap_package_node_manager(package));

				/* if the region is a torso, and loaded a default torso, set it's fit field values */
				if ((region->type==TORSO)&&(get_unemap_package_torso_group_name(package)))
				{
					set_torso_fit_field_values(map_3d_package);
				}
			}
			else
			{
				if (map_3d_package)
				{
					/*this will do a deaccess*/
					set_Region_map_3d_package(region,(struct Map_3d_package *)NULL);
				}
				/* rebuild  map & node and element groups */
				interpolation_function_to_node_group(&node_order_info,field_order_info,
					function,package,fit_name,sock_lambda,torso_major_r,
					torso_minor_r,patch_z,region);
				make_fit_elements(fit_name,node_order_info,field_order_info,
					get_unemap_package_element_group_manager(package),
					get_unemap_package_basis_manager(package),
					get_unemap_package_element_manager(package),
					function->region_type,function->number_of_rows,
					function->number_of_columns,
					function->x_mesh,function->y_mesh,region);
				/* if have loaded  a default_torso and if the region is a torso*/
				if ((get_unemap_package_torso_group_name(package))&&(region->type==TORSO))
				{
					/*  set up the mapped torso node and element groups */
					make_mapped_torso_node_and_element_groups(region,package);
					/* map_3d_package just created in interpolation_function_to_node_group*/
					map_3d_package=get_Region_map_3d_package(region);
					/* set it's fit field values */
					set_torso_fit_field_values(map_3d_package);
				}
			}
		}/* if (ALLOCATE( */
		else
		{
			display_message(ERROR_MESSAGE,"make_fit_node_and_element_groups."
			"make_fit_name failed");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_fit_node_and_element_groups."
			" Invalid argument(s)");
		return_code=0;
	}
	DEALLOCATE(fit_name);
	LEAVE;
	
	return (return_code);
}/*make_fit_node_and_element_groups */

#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int map_draw_constant_thickness_contours(struct Scene *scene,
	struct Map_drawing_information *map_drawing_information,
	struct Computed_field *data_field,
	int number_of_contours,FE_value contour_minimum,FE_value contour_maximum,
	struct Region *region,int default_torso_loaded,int delaunay_map)
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
Removes any existing contours, draw <number_of_contours> map contours, evenly
spaced between <contour_minimum> and <contour_maximum>. If <number_of_contour>
=0, simply removes any existing contours.
==============================================================================*/
{
	int i,old_number_of_contours,return_code;
	struct Colour colour;
	struct Graphical_material *contour_material,*default_selected_material;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct GROUP(FE_element) *element_group;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings **contour_settings,**old_contour_settings;
	struct Map_3d_package *map_3d_package=(struct Map_3d_package *)NULL;
	FE_value contour_step,contour_value;

	ENTER(map_make_surfaces);
	contour_settings=(struct GT_element_settings **)NULL;
	old_contour_settings=(struct GT_element_settings **)NULL;
	gt_element_group=(struct GT_element_group *)NULL;
	contour_material=(struct Graphical_material *)NULL;
	default_selected_material=(struct Graphical_material *)NULL;
	graphical_material_manager=(struct MANAGER(Graphical_material) *)NULL;
	element_group=(struct GROUP(FE_element) *)NULL;
	if ((!number_of_contours)||(data_field&&scene&&map_drawing_information&&region))
	{
		map_3d_package=get_Region_map_3d_package(region);
		/*non-current regions will have had map_3d_package set to NULL by */
		/* set_Region_map_3d_package(NULL) in draw_map_3d */
		if (map_3d_package)
		{
			if (delaunay_map) /* the interpolation_type, stored in the map?*/
			{
				/* do contour on delaunay*/
				element_group=get_map_3d_package_delaunay_torso_element_group(map_3d_package);
			}
			else
			{
				if (default_torso_loaded&&(region->type==TORSO))
				{
					/* do contour on torso surface*/
					element_group=get_map_3d_package_mapped_torso_element_group(map_3d_package);
				}
				else
				{
					/* do contour on cylinder surface */
					element_group=get_map_3d_package_element_group(map_3d_package);
				}
			}
			/* not an error if no element group, this is case with NO_INTERPOLATION, ie no surface*/
			if (element_group)
			{
				graphical_material_manager=
					get_map_drawing_information_Graphical_material_manager(map_drawing_information);
				gt_element_group=Scene_get_graphical_element_group(scene,element_group);
				default_selected_material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)
					("default_selected",graphical_material_manager);
				/* do nothing if the number of contours is the same */
				get_map_3d_package_contours(map_3d_package,&old_number_of_contours,
					&old_contour_settings);
				/* create material for the contour, so can colour it black, indep of the  */
				/* map surface (default) material*/
				MANAGER_BEGIN_CACHE(Graphical_material)(graphical_material_manager);
				if (!(contour_material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)
					("contour",graphical_material_manager)))
				{
					if (contour_material=CREATE(Graphical_material)(
						"contour"))
					{
						colour.red=0.0;
						colour.green=0.0;
						colour.blue=0.0;
						Graphical_material_set_ambient(contour_material,&colour);
						Graphical_material_set_diffuse(contour_material,&colour);
						ADD_OBJECT_TO_MANAGER(Graphical_material)(contour_material,
							graphical_material_manager);
					}
				}
				MANAGER_END_CACHE(Graphical_material)(graphical_material_manager);
				/* remove the old contour settings from the gt_element_group */
				if (old_number_of_contours)
				{
					for(i=0;i<old_number_of_contours;i++)
					{
						GT_element_group_remove_settings(gt_element_group,old_contour_settings[i]);
					}
				}
				free_map_3d_package_map_contours(map_3d_package);
				if (number_of_contours)
				{
					/* calculate the contour intervals */
					contour_value=contour_minimum;
					contour_step=(contour_maximum-contour_minimum)/(number_of_contours-1);
					/* allocate, define and set the contours */
					ALLOCATE(contour_settings,struct GT_element_settings *,number_of_contours);
					for(i=0;i<number_of_contours;i++)
					{
						contour_settings[i]=CREATE(GT_element_settings)
							(GT_ELEMENT_SETTINGS_ISO_SURFACES);
						GT_element_settings_set_material(contour_settings[i],contour_material);
						GT_element_settings_set_selected_material(contour_settings[i],
							default_selected_material);
						GT_element_settings_set_iso_surface_parameters(contour_settings[i],
							data_field,contour_value);
						GT_element_group_add_settings(gt_element_group,contour_settings[i],0);
						contour_value+=contour_step;
					}
				}
				set_map_3d_package_contours(map_3d_package,number_of_contours,
					contour_settings);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_draw_constant_thickness_contours. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_draw_constant_thickness_contours */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int map_show_surface(struct Scene *scene,
	struct GROUP(FE_element) *element_group,struct Graphical_material *material,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Spectrum *spectrum,struct Computed_field *data_field,
	struct Colour *no_interpolation_colour, struct Time_object *time_object,
	struct User_interface *user_interface,int direct_interpolation,
	int autoranging_spectrum,struct Texture *skin_texture,
	struct Computed_field *skin_texture_coords)
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Adds map's surfaces in the given <material> to the graphical finite element for
<element_group> on <scene>. Regenerates the GFE but does not update the scene.
If  <spectrum> and <data_field> are set, and <no_interpolation_colour>  is NULL,
applies <spectrum> and <data_field> these to the <material>.
If  <spectrum> and <data_field> are NULL and <no_interpolation_colour> is set,
removes any existing <spectrum> and <data_field>  and
applies  <no_interpolation_colour> to the <material>.
if <direct_interpolation> is true, sets the element discretization to 1
Also applies <number_of_contours> contours to surface.
==============================================================================*/
{
	int existing_autorange_spectrum_flag,maximum_discretization,
		required_discretization,return_code,update_settings;
	struct Colour white;
	struct Element_discretization discretization;
	struct GT_element_group *gt_element_group;
	struct Computed_field *existing_data_field,*new_data_field,
		*new_skin_texture_coords;
	struct Graphical_material *default_selected_material,
		*material_copy;
	struct GT_element_settings *old_settings,*new_settings;
	struct Scene_object *scene_object;
	struct Spectrum *existing_spectrum;
	struct Texture *new_skin_texture;

	ENTER(map_make_surfaces);
	new_data_field=( struct Computed_field *)NULL;
	new_skin_texture_coords=( struct Computed_field *)NULL;
	new_skin_texture=(struct Texture *)NULL;
	gt_element_group=(struct GT_element_group *)NULL;
	old_settings=(struct GT_element_settings *)NULL;
	new_settings=(struct GT_element_settings *)NULL;
	material_copy=(struct Graphical_material *)NULL;
	default_selected_material=(struct Graphical_material *)NULL;
	existing_data_field=(struct Computed_field *)NULL;
	existing_spectrum=(struct Spectrum *)NULL;
	if (scene&&element_group&&material&&user_interface&&
		((spectrum&&data_field&&!no_interpolation_colour&&!skin_texture&&!skin_texture_coords)
		||(!spectrum&&!data_field&&no_interpolation_colour&&!skin_texture&&!skin_texture_coords)
			||(spectrum&&!data_field&&direct_interpolation&&skin_texture&&skin_texture_coords)))
	{
		if ((scene_object=Scene_get_scene_object_with_element_group(scene,element_group))&&
			(gt_element_group=Scene_object_get_graphical_element_group(scene_object))&&
			(default_selected_material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)
			("default_selected",graphical_material_manager)))
		{
			white.red=1.0;
			white.green=1.0;
			white.blue=1.0;
			GT_element_group_get_element_discretization(gt_element_group,
				&discretization);
			if ((direct_interpolation)&&(!skin_texture))
			{
				required_discretization=1;
			}
			else
			{
				read_element_discretization_defaults(&required_discretization,
					&maximum_discretization,user_interface);
			}
			if (!((discretization.number_in_xi1==required_discretization)&&
				(discretization.number_in_xi2==required_discretization)&&
				(discretization.number_in_xi3==required_discretization)))
			{
				discretization.number_in_xi1=required_discretization;
				discretization.number_in_xi2=required_discretization;
				discretization.number_in_xi3=required_discretization;
				GT_element_group_set_element_discretization(gt_element_group,&discretization,
					user_interface);
			}
			MANAGER_BEGIN_CACHE(Graphical_material)(graphical_material_manager);
			/* do we already have settings, ie already created graphical element group?*/
			if (old_settings=first_settings_in_GT_element_group_that(gt_element_group,
				GT_element_settings_type_matches,(void *)GT_ELEMENT_SETTINGS_SURFACES))
			{
				return_code=1;
				GT_element_settings_get_data_spectrum_parameters(old_settings,&existing_data_field,
					&existing_spectrum);
				existing_autorange_spectrum_flag =
					GT_element_settings_get_autorange_spectrum_flag(old_settings);
				update_settings = 0;
				if (skin_texture)
				{
					update_settings = 1;
				}
				else
				{
					/* change the spectrum, if it's different from the existing one */
					if (!((data_field==existing_data_field)&&(spectrum==existing_spectrum)&&
						(existing_autorange_spectrum_flag==autoranging_spectrum)))
					{
						update_settings = 1;
					}
				}
			}/* if (old_settings=f*/
			if ((old_settings&&update_settings)||(!old_settings))
			{
				new_settings=CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
				material_copy=CREATE(Graphical_material)("");
				if (new_settings&&material_copy&&
					MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name)
					(material_copy,material))
				{
					if (data_field&&spectrum)
					{
						/*remove any skin texture*/
						new_skin_texture=(struct Texture *)NULL;
						new_skin_texture_coords=( struct Computed_field *)NULL;
						new_data_field=data_field;
						Graphical_material_set_ambient(material_copy,&white);
						Graphical_material_set_diffuse(material_copy,&white);
					}
					else if (skin_texture)
					{
						/* set the skin texture*/
						new_skin_texture=skin_texture;
						new_skin_texture_coords=skin_texture_coords;
						new_data_field=( struct Computed_field *)NULL;
						Graphical_material_set_ambient(material_copy,&white);
						Graphical_material_set_diffuse(material_copy,&white);
					}
					else
					{
						/*remove any skin texture*/
						new_skin_texture=(struct Texture *)NULL;
						new_skin_texture_coords=( struct Computed_field *)NULL;
						new_data_field=( struct Computed_field *)NULL;
						Graphical_material_set_ambient(material_copy,no_interpolation_colour);
						Graphical_material_set_diffuse(material_copy,no_interpolation_colour);
					}
					/* set the skin texture*/
					Graphical_material_set_texture(material_copy,new_skin_texture);
					if (MANAGER_MODIFY_NOT_IDENTIFIER(Graphical_material,name)(
						material,material_copy,graphical_material_manager))
					{
						GT_element_settings_set_texture_coordinate_field(new_settings,
							new_skin_texture_coords);
						GT_element_settings_set_material(new_settings,material);
						GT_element_settings_set_selected_material(new_settings,
							default_selected_material);
						GT_element_settings_set_autorange_spectrum_flag(new_settings,
							autoranging_spectrum);
						GT_element_settings_set_data_spectrum_parameters(new_settings,
							new_data_field,spectrum);
						if (old_settings)
						{
							GT_element_group_modify_settings(gt_element_group,old_settings,
							new_settings);
						}
						else
						{
							if (!GT_element_group_add_settings(gt_element_group,new_settings, 0))
							{
								DESTROY(GT_element_settings)(&new_settings);
								return_code = 0;
							}
						}
						/* Do this after the appropriate stuff has been put in the GT_element_group */
						Scene_object_set_time_object(scene_object, time_object);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"map_show_surface. MANAGER_MODIFY failed ");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"map_show_surface. Couldn't copy material ");
					return_code=0;
				}
			}/* (update_settings) */
			MANAGER_END_CACHE(Graphical_material)(graphical_material_manager);
		}
		else
		{
			display_message(ERROR_MESSAGE,
						"map_show_surface. no gt_element_group or default_material ");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_show_surface.  Invalid argument(s)");
		return_code=0;
	}
	if (material_copy)
	{
		DESTROY(Graphical_material)(&material_copy);
	}
	LEAVE;
	
	return (return_code);
} /* map_show_surface */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
#define FIT_SOCK_LAMBDA 4.5 /* chosen by trial and error!*/
#define FIT_SOCK_FOCUS 1.0
#if defined (OLD_CODE)
#define FIT_TORSO_MAJOR_R 200.0
#define FIT_TORSO_MINOR_R 100.0
#endif /* defined (OLD_CODE) */
#if defined (ROUND_TORSO)
#define FIT_TORSO_MAJOR_R 50.0
#define FIT_TORSO_MINOR_R 0.0
#else /* defined (ROUND_TORSO) */
/*this is for FOR AJPs p117_m8c1_modified.signal*/
#define FIT_TORSO_MAJOR_R 100.0
#define FIT_TORSO_MINOR_R 50.0
#endif /* defined (ROUND_TORSO) */
#define FIT_PATCH_Z 0.0
static int make_and_add_map_electrode_position_field(
	struct Unemap_package *unemap_package,
	struct GROUP(FE_node) *rig_node_group,struct Region *region,int delaunay_map)
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
if  necessaty makes maps electrode position field, and adds to the nodes in the
<rig_node_group> <electrode_position_field> used to match the created
map_electode_position field
==============================================================================*/
{
	enum FE_field_type field_type;
	enum Region_type region_type;
	enum Value_type time_value_type,value_type;
	char **component_names;
	char *field_name=(char *)NULL;
	char *region_type_str=(char *)NULL;
	char *patch_str = "map_patch";
	char *position_str = "_electrode_position";
	char *sock_str = "map_sock";
	char *torso_str = "map_torso";
	enum CM_field_type cm_field_type;
	int i,number_of_components,number_of_indexed_values,number_of_times,
		return_code,string_length;
	struct Coordinate_system *coordinate_system=(struct Coordinate_system *)NULL;
	struct FE_field *map_electrode_position_field=(struct FE_field *)NULL;
	struct FE_field *map_position_field=(struct FE_field *)NULL;
	struct FE_field *indexer_field=(struct FE_field *)NULL;
	struct MANAGER(FE_field) *field_manager=(struct MANAGER(FE_field) *)NULL;
	struct Map_3d_package *map_3d_package=(struct Map_3d_package *)NULL;
	struct FE_time *fe_time;
	component_names=(char **)NULL;

	ENTER(make_and_add_map_electrode_position_fields);
	if (unemap_package&&rig_node_group&&region)
	{
		/* if delaunay, just use the electrode_position_field */
		if (delaunay_map)
		{
			map_electrode_position_field=get_Region_electrode_position_field(region);
			set_Region_map_electrode_position_field(region,map_electrode_position_field);
			return_code=1;
		}
		else
		{
			return_code=1;
			region_type=region->type;
			/* compose the field name*/
			switch (region_type)
			{
				case SOCK:
				{
					region_type_str = sock_str;
				} break;
				case TORSO:
				{
					region_type_str = torso_str;
				} break;
				case PATCH:
				{
					region_type_str = patch_str;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"make_and_add_map_electrode_position_fields. Bad region type");
					return_code=0;
				} break;
			}
			if (return_code)
			{
				string_length = strlen(region_type_str);
				string_length += strlen(position_str);
				string_length++;
				if (ALLOCATE(field_name,char,string_length))
				{
					strcpy(field_name,region_type_str);
					strcat(field_name,position_str);
					/* get the map position field,  */
					map_3d_package=get_Region_map_3d_package(region);
					map_position_field=get_map_3d_package_position_field(map_3d_package);
					field_manager=get_unemap_package_FE_field_manager(unemap_package);
					fe_time=get_unemap_package_FE_time(unemap_package);
					/* assemble all info for get_FE_field_manager_matched_field */
					/* ??JW what if map_position_field hasn't been created?  */
					cm_field_type=get_FE_field_CM_field_type(map_position_field);
					if ((coordinate_system=get_FE_field_coordinate_system(
						map_position_field))&&(number_of_components=
						get_FE_field_number_of_components(map_position_field))&&
						(ALLOCATE(component_names,char *,number_of_components)))
					{
						number_of_times=get_FE_field_number_of_times(map_position_field);
						value_type=get_FE_field_value_type(map_position_field);
						field_type=get_FE_field_FE_field_type(map_position_field);
						time_value_type=get_FE_field_time_value_type(map_position_field);
						for(i=0;i<number_of_components;i++)
						{
							*component_names=get_FE_field_component_name(map_position_field,i);
							component_names++;
						}
						component_names-=number_of_components;
						if (field_type==INDEXED_FE_FIELD)
						{
							get_FE_field_type_indexed(map_position_field,&indexer_field,
								&number_of_indexed_values);
						}
						else
						{
							indexer_field=(struct FE_field *)NULL;
							number_of_indexed_values=0;
						}
						/* find or create the new field in the fe_field_manager */
						if ((map_electrode_position_field=
							get_FE_field_manager_matched_field(field_manager,field_name,
							field_type,fe_time,indexer_field,number_of_indexed_values,
							cm_field_type,coordinate_system,value_type,
							number_of_components,component_names,number_of_times,
							time_value_type,(struct FE_field_external_information *)NULL)))
						{
							struct FE_field *old_map_electrode_position_field;
							old_map_electrode_position_field=
								get_Region_map_electrode_position_field(region);
							if (old_map_electrode_position_field!=map_electrode_position_field)
							{
								/* add it to the region, so can use it later */
								set_Region_map_electrode_position_field(region,
									map_electrode_position_field);
								/* add a map_electrode_position_field to the nodes. */
								rig_node_group_add_map_electrode_position_field(unemap_package,
									rig_node_group,map_electrode_position_field);
								/* Set the lambda/r/z to match  the fitted surface's. Should really*/
								/* do vice versa, ie  set surface to electrodes */
								rig_node_group_set_map_electrode_position_lambda_r(unemap_package,
									rig_node_group,region,FIT_SOCK_LAMBDA,FIT_TORSO_MAJOR_R,
									FIT_TORSO_MINOR_R);
							}
						}/* if (ADD_OBJECT_TO_MANAGER(FE_field) */
						else
						{
							display_message(ERROR_MESSAGE,"make_and_add_map_electrode_position_fields."
								" error with map_electrode_position_field to manager");
							return_code=0;
							DESTROY(FE_field)(&map_electrode_position_field);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"make_and_add_map_electrode_position_fields."
							"error getting field info ");
						return_code=0;
						DESTROY(FE_field)(&map_electrode_position_field);
					}
					if (component_names)
					{
						DEALLOCATE(component_names);
					}
				}/* if (ALLOCATE(field_name,char,string_length)) */
				else
				{
					display_message(ERROR_MESSAGE,"make_and_add_map_electrode_position_fields."
						"Out of memory ");
					return_code=0;
				}
			}/* if (return_code)*/
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_and_add_map_electrode_position_fields. "
			"Invalid argument(s)");
		return_code=0;
	}
	DEALLOCATE(field_name);
	LEAVE;
	
	return (return_code);
} /* make_and_add_map_electrode_position_fields */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct GT_object *map_get_map_electrodes_glyph(
	struct Map_drawing_information *drawing_information,
	struct Map_3d_package *map_3d_package,enum Electrodes_marker_type electrodes_marker_type)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Gets  and returns a <glyph> for the map electrode, based upon <electrodes_marker_type>
Also sets the glyph in the <map_3d_package>
==============================================================================*/
{
	struct GT_object *glyph;

	ENTER(map_get_map_electrodes_glyph);
	glyph=(struct GT_object *)NULL;
	if (drawing_information)
	{
		/* match the names to the marker types. Maybe store the marker types ??JW*/
		/* see also map_update_map_electrodes */
		switch (electrodes_marker_type)
		{
			case CIRCLE_ELECTRODE_MARKER:
			{
				glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("sphere",
					get_map_drawing_information_glyph_list(drawing_information));
			} break;
			case PLUS_ELECTRODE_MARKER:
			{
				glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("cross",
					get_map_drawing_information_glyph_list(drawing_information));
			} break;
			case SQUARE_ELECTRODE_MARKER:
			{
				glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("diamond",
					get_map_drawing_information_glyph_list(drawing_information));
			} break;
			case HIDE_ELECTRODE_MARKER:
			{
				glyph=(struct GT_object *)NULL;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"map_get_map_electrodes_glyph.\n "
					"invalide glyph type ");glyph=(struct GT_object *)NULL;
			} break;
		}
		if (glyph)
		{
			set_map_3d_package_electrode_glyph(map_3d_package,glyph);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"map_get_map_electrodes_glyph. "
			"Invalid argument(s)");
	}
	LEAVE;
	
	return (glyph);
}/* map_get_map_electrodes_glyph */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (OLD_CODE)
#if defined (UNEMAP_USE_3D)
static int map_update_electrode_colour_from_time(struct Unemap_package *package,
	FE_value time)
/*******************************************************************************
LAST MODIFIED : 1 May 2000

DESCRIPTION :
Changes the time in the computed field time_field.
NOTE: This will cause a rebuild of the electrode glyph graphics object
(with any associated messages). This is in addition to the update cause by rebuilding
the surface.
==============================================================================*/
{
	int return_code;
	struct Computed_field *time_field,*time_field_copy;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(map_update_electrode_colour_from_time);
	time_field=(struct Computed_field *)NULL;
	time_field_copy=(struct Computed_field *)NULL;
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	if (package&&(computed_field_manager=
		get_unemap_package_Computed_field_manager(package)))
	{
		return_code=1;
		/* check if time_field exists */
		if (!(time_field=get_unemap_package_time_field(package)))
		{
			display_message(ERROR_MESSAGE,
			"map_update_electrode_colour_from_time. No time field to alter");
			return_code=0;
		}
		/* alter existing time_field*/
		else
		{
			if (time_field_copy=CREATE(Computed_field)("signal_time_copy"))
			{
				MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name)(time_field_copy,
					time_field );
				Computed_field_set_type_constant(time_field_copy,1,&time);
				MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name)(time_field,
					time_field_copy,computed_field_manager);
				DESTROY(Computed_field)(&time_field_copy);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_update_electrode_colour_from_time  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return return_code;
}/* map_updatelectrode_colour_from_time */
#endif /* (UNEMAP_USE_3D) */
#endif /* defined (OLD_CODE) */

#if defined (UNEMAP_USE_3D)
static int map_define_scaled_offset_signal_at_time(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Gets or creates the computed fields necessary for the map colour.
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_gain_field,*computed_offset_field,
		*offset_signal_value_at_time_field,*scaled_offset_signal_value_at_time_field,
		*signal_value_at_time_field;
	struct FE_field *channel_gain_field,*channel_offset_field,*signal_field;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(map_define_scaled_offset_signal_at_time);
	signal_value_at_time_field=(struct Computed_field *)NULL;
	computed_gain_field=(struct Computed_field *)NULL;
	computed_offset_field=(struct Computed_field *)NULL;
	offset_signal_value_at_time_field=(struct Computed_field *)NULL;
	scaled_offset_signal_value_at_time_field=(struct Computed_field *)NULL;
	signal_field=(struct FE_field *)NULL;
	channel_gain_field=(struct FE_field *)NULL;
	channel_offset_field=(struct FE_field *)NULL;
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	if (package&&(computed_field_manager=get_unemap_package_Computed_field_manager(package))&&
		(signal_field=get_unemap_package_signal_field(package))
		&&(channel_gain_field=get_unemap_package_channel_gain_field(package))
		&&(channel_offset_field=get_unemap_package_channel_offset_field(package)))
	{
		return_code=1;
		computed_gain_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_read_only_with_fe_field,(void *)(channel_gain_field),
			computed_field_manager);
		computed_offset_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_read_only_with_fe_field,(void *)(channel_offset_field),
			computed_field_manager);
		/* set up signal_value_at_time_field for spectrum according to signal values and time */
		MANAGER_BEGIN_CACHE(Computed_field)(computed_field_manager);
		/* create new data_field,using time_field*/
		/*signal value at time*/
		if (!(signal_value_at_time_field=get_unemap_package_signal_value_at_time_field
			(package)))
		{
			/* Fetch the wrapper fo the fe_field from the manager */
			if (signal_value_at_time_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_read_only_with_fe_field,(void *)
				(signal_field),computed_field_manager),get_unemap_package_FE_field_manager(package))
			{
				set_unemap_package_signal_value_at_time_field(package,signal_value_at_time_field);
			}
		}
		/*offset the signal value*/
		if (!(offset_signal_value_at_time_field=get_unemap_package_offset_signal_value_at_time_field
			(package)))
		{
			if (offset_signal_value_at_time_field=CREATE(Computed_field)("offset_signal_value_at_time"))
			{
				if (!((Computed_field_set_type_add(offset_signal_value_at_time_field,
					signal_value_at_time_field,1,computed_offset_field,-1))&&
					(ADD_OBJECT_TO_MANAGER(Computed_field)(offset_signal_value_at_time_field,
						computed_field_manager))&&
					set_unemap_package_offset_signal_value_at_time_field(package,
						offset_signal_value_at_time_field)))
				{
					DESTROY(Computed_field)(&offset_signal_value_at_time_field);
				}
			}
		}
		/* scale the signal value by the gain */
		if (!(scaled_offset_signal_value_at_time_field=
			get_unemap_package_scaled_offset_signal_value_at_time_field(package)))
		{
			if (scaled_offset_signal_value_at_time_field=CREATE(Computed_field)
				("scaled_offset_signal_value_at_time"))
			{
				if (!((Computed_field_set_type_dot_product(scaled_offset_signal_value_at_time_field,
					offset_signal_value_at_time_field,computed_gain_field))&&
					(ADD_OBJECT_TO_MANAGER(Computed_field)(scaled_offset_signal_value_at_time_field,
						computed_field_manager))&&
					set_unemap_package_scaled_offset_signal_value_at_time_field(package,
						scaled_offset_signal_value_at_time_field)))
				{
					DESTROY(Computed_field)(&scaled_offset_signal_value_at_time_field);
				}
			}
		}
		MANAGER_END_CACHE(Computed_field)(computed_field_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_define_scaled_offset_signal_at_time  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return return_code;
}/* map_define_scaled_offset_signal_at_time */
#endif /* (UNEMAP_USE_3D) */


#if defined (UNEMAP_USE_3D)
static int map_set_electrode_colour_from_time(struct Unemap_package *package,
	struct Spectrum *spectrum,struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Gets or creates the computed fields necessary for the map colour based on
the scaled_offset_signal and assigns that field to the <settings>.
==============================================================================*/
{
	int return_code;
	struct Computed_field *scaled_offset_signal_value_at_time_field;

	ENTER(map_set_electrode_colour_from_time);
	if (package&&spectrum&&settings)
	{
		return_code=1;
		if (!(scaled_offset_signal_value_at_time_field =
			get_unemap_package_scaled_offset_signal_value_at_time_field(package)))
		{
			map_define_scaled_offset_signal_at_time(package);
			scaled_offset_signal_value_at_time_field =
				get_unemap_package_scaled_offset_signal_value_at_time_field(package);
		}
		/* alter the spectrum settings with the data */
		GT_element_settings_set_data_spectrum_parameters(settings,
			scaled_offset_signal_value_at_time_field,spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_set_electrode_colour_from_time  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return return_code;
}/* map_set_electrode_colour_from_time */
#endif /* (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int map_show_map_electrodes(struct Unemap_package *package,
	struct GT_object *glyph,struct Map *map,struct Region *region)
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
Construct the settings and build the graphics objects for the glyphs.
==============================================================================*/
{
	char *group_name;
	enum Glyph_scaling_mode glyph_scaling_mode;
	int return_code;
	struct Computed_field *computed_coordinate_field,*computed_field,*label_field,
		*orientation_scale_field, *variable_scale_field;
	struct FE_field *map_electrode_position_field,*field;
	struct Graphical_material *electrode_selected_material,*electrode_material;
	struct GROUP(FE_element) *rig_element_group;
	struct GROUP(FE_node) *unrejected_node_group;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *unselected_settings,*selected_settings;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct Map_3d_package *map_3d_package=(struct Map_3d_package *)NULL;
	struct Map_drawing_information *drawing_information=
		(struct Map_drawing_information *)NULL;
	struct Scene *scene;
	struct Scene_object *scene_object;
	Triple glyph_centre,glyph_size,glyph_scale_factors;

	ENTER(map_show_map_electrodes);
	orientation_scale_field=(struct Computed_field *)NULL;
	computed_coordinate_field=(struct Computed_field *)NULL;
	gt_element_group=(struct GT_element_group *)NULL;
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	electrode_material=(struct Graphical_material *)NULL;
	electrode_selected_material=(struct Graphical_material *)NULL;
	scene=(struct Scene *)NULL;
	unrejected_node_group=(struct GROUP(FE_node) *)NULL;
	rig_element_group=(struct GROUP(FE_element) *)NULL;
	group_name=(char *)NULL;
	map_electrode_position_field=(struct FE_field *)NULL;
	element_group_manager=(struct MANAGER(GROUP(FE_element)) *)NULL;
	unselected_settings=(struct GT_element_settings *)NULL;
	selected_settings=(struct GT_element_settings *)NULL;
	graphical_material_manager=(struct MANAGER(Graphical_material) *)NULL;
	field=(struct FE_field *)NULL;
	computed_field=(struct Computed_field *)NULL;
	scene_object=(struct Scene_object *)NULL;
	if (map&&(drawing_information=map->drawing_information)&&
		region&&package&&glyph&&(electrode_material=
		get_map_drawing_information_electrode_graphical_material(drawing_information))&&
		(scene=get_map_drawing_information_scene(drawing_information))&&
		(computed_field_manager=
			get_unemap_package_Computed_field_manager(package))&&
		(element_group_manager=
			get_unemap_package_element_group_manager(package))&&
		(unrejected_node_group=get_Region_unrejected_node_group(region))&&
		(map_electrode_position_field=
			get_Region_map_electrode_position_field(region)))
	{
		return_code=0;
		GET_NAME(GROUP(FE_node))(unrejected_node_group,&group_name);
		rig_element_group=
			FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
			(group_name,element_group_manager);
		if (rig_element_group&&(scene_object=Scene_get_scene_object_with_element_group
			(scene,rig_element_group))&&(gt_element_group=
			Scene_object_get_graphical_element_group(scene_object)))
		{
			/* do nothing if already have these settings in this group*/
			if (!(unselected_settings=first_settings_in_GT_element_group_that(gt_element_group,
				GT_element_settings_type_matches,(void *)GT_ELEMENT_SETTINGS_NODE_POINTS)))
			{
				if ((unselected_settings=CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_NODE_POINTS))&&
						(selected_settings=CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_NODE_POINTS)))
				{
					graphical_material_manager=
						get_map_drawing_information_Graphical_material_manager(drawing_information);
					electrode_selected_material=FIND_BY_IDENTIFIER_IN_MANAGER
						(Graphical_material,name)("electrode_selected",graphical_material_manager);
					GT_element_settings_set_selected_material(unselected_settings,electrode_selected_material);
					GT_element_settings_set_selected_material(selected_settings,electrode_selected_material);
					GT_element_settings_set_material(unselected_settings,electrode_material);
					GT_element_settings_set_material(selected_settings,electrode_material);
					/* use selected settings when selected, unselected when unselected(!) */
					GT_element_settings_set_select_mode(unselected_settings,GRAPHICS_DRAW_UNSELECTED);
					GT_element_settings_set_select_mode(selected_settings,GRAPHICS_DRAW_SELECTED);
					glyph_scaling_mode = GLYPH_SCALING_GENERAL;
					glyph_centre[0]=0.0;
					glyph_centre[1]=0.0;
					glyph_centre[2]=0.0;
					glyph_size[0]=map->electrodes_marker_size;
					glyph_size[1]=map->electrodes_marker_size;
					glyph_size[2]=map->electrodes_marker_size;
					orientation_scale_field=(struct Computed_field *)NULL;
					glyph_scale_factors[0]=1.0;
					glyph_scale_factors[1]=1.0;
					glyph_scale_factors[2]=1.0;
					variable_scale_field=(struct Computed_field *)NULL;
					computed_field_manager=get_unemap_package_Computed_field_manager(package);
					computed_coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						Computed_field_is_read_only_with_fe_field,(void *)
						(map_electrode_position_field),computed_field_manager);
					GT_element_settings_set_coordinate_field(unselected_settings,
						computed_coordinate_field);
					GT_element_settings_set_coordinate_field(selected_settings,
						computed_coordinate_field);
					GT_element_settings_set_glyph_parameters(unselected_settings, glyph,
						glyph_scaling_mode, glyph_centre, glyph_size, orientation_scale_field,
						glyph_scale_factors, variable_scale_field);
					GT_element_settings_set_glyph_parameters(selected_settings, glyph,
						glyph_scaling_mode, glyph_centre, glyph_size, orientation_scale_field,
						glyph_scale_factors, variable_scale_field);
					if (map->colour_electrodes_with_signal)
					/* else electrode takes colour of electrode_material*/
					{
						map_set_electrode_colour_from_time(package,map->drawing_information->spectrum,
							unselected_settings);
					}
					switch (map->electrodes_label_type)
					{
						case SHOW_ELECTRODE_VALUES:
						{
							label_field=get_unemap_package_scaled_offset_signal_value_at_time_field
								(package);
						} break;
						case SHOW_ELECTRODE_NAMES:
						{
							field=get_unemap_package_device_name_field(package);
							if (field)
							{
								computed_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
									Computed_field_is_read_only_with_fe_field,
									(void *)(field),computed_field_manager);
								if (computed_field)
								{
									label_field=computed_field;
								}
							}
						} break;
						case SHOW_CHANNEL_NUMBERS:
						{
							field=get_unemap_package_channel_number_field(package);
							if (field)
							{
								computed_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
									Computed_field_is_read_only_with_fe_field,
									(void *)(field),computed_field_manager);
								if (computed_field)
								{
									label_field=computed_field;
								}
							}
						} break;
						case HIDE_ELECTRODE_LABELS:
						{
							label_field=(struct Computed_field *)NULL;
						} break;
					}/* switch (electrodes_label_type) */
					GT_element_settings_set_label_field(unselected_settings,label_field);
					GT_element_settings_set_label_field(selected_settings,label_field);
					/* add the settings to the group */
					if ((GT_element_group_add_settings(gt_element_group,unselected_settings,1))&&
							(GT_element_group_add_settings(gt_element_group,selected_settings,2)))
					{
						map_3d_package=get_Region_map_3d_package(region);
					set_map_3d_package_electrode_size(map_3d_package,map->electrodes_marker_size);
						set_map_3d_package_electrodes_label_type(map_3d_package,map->electrodes_label_type);
						set_map_3d_package_colour_electrodes_with_signal(map_3d_package,
							map->colour_electrodes_with_signal);
						return_code=1;
					}/* if (GT_element_group_add_settings( */
					else
					{
						DESTROY(GT_element_settings)(&unselected_settings);
						DESTROY(GT_element_settings)(&selected_settings);
					}
				}/* if (settings=CREATE(GT_element_settings) */
				/* Do this after the settings have been added */
				Scene_object_set_time_object(scene_object,
					get_unemap_package_potential_time_object(package));
			}/* if (!(settings=first_settings_in_GT_element_group_that */
			else
			{
				/* nothing to do */
				return_code=1;
			}
		}/* if (rig_element_group&&*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_show_map_electrodes. Invalid argument(s)");
		return_code=0;
	}
	DEALLOCATE(group_name);
	LEAVE;
	
	return (return_code);
} /* map_show_map_electrodes */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int map_remove_map_electrode_glyphs(
	struct Map_drawing_information *drawing_information,
	struct Unemap_package *package,
	struct Map_3d_package *map_3d_package,struct GROUP(FE_node) *rig_node_group)
/*******************************************************************************
LAST MODIFIED : 22 October 1999

DESCRIPTION :
Removes the map electrode glyphs , for each node in the rig_node_group
in <region>
==============================================================================*/
{
	int return_code;

	ENTER(map_remove_map_electrode_glyphs);
	if (package&&rig_node_group&&drawing_information)
	{
		free_unemap_package_rig_node_group_glyphs(drawing_information,package,rig_node_group);
		if (map_3d_package)
		{
			set_map_3d_package_electrode_glyph(map_3d_package,(struct GT_object *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_remove_map_electrode_glyphs . Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}/* map_remove_map_electrode_glyphs */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int map_update_map_electrodes(struct Unemap_package *package,
	struct Region *region,struct Map *map)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
if necessary,create glyphs for the map electrodes, for each node in the
rig_node_group in <region>. Glyph type taken from
<electrodes_marker_type>
==============================================================================*/
{
	int return_code;
	struct GT_object *electrode_glyph=(struct GT_object *)NULL;
	struct Map_3d_package *map_3d_info=(struct Map_3d_package *)NULL;
	ENTER(map_update_map_electrodes);
	if (package&&region&&map)
	{
		/* get the glyph*/
		if ((map_3d_info=get_Region_map_3d_package(region))&&
			!(electrode_glyph=get_map_3d_package_electrode_glyph(map_3d_info)))
		{
			electrode_glyph=map_get_map_electrodes_glyph(map->drawing_information,
				map_3d_info,map->electrodes_marker_type);
		}
		/* glyph can be NULL for no markers */
		if (electrode_glyph)
		{
			map_show_map_electrodes(package,electrode_glyph,map,region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_update_map_electrodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_update_map_electrodes */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct Computed_field *map_get_data_field_for_3d_map(struct Map *map,
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 5 February 2002

DESCRIPTION :
Uses the enumerators in the <map> to determine what the currently displayed
data field is.
==============================================================================*/
{
	struct Computed_field *data_field;
	struct FE_field *fit_field;
	struct Unemap_package *unemap_package;

	ENTER(map_get_data_field_for_3d_map);
	if (map && (unemap_package=map->unemap_package))
	{
		if ((map->interpolation_type==NO_INTERPOLATION)||
			(map->colour_option==HIDE_COLOUR))
		{
			data_field = (struct Computed_field *)NULL;
		}
		else
		{
			/* Get the map "fit" field, to use for the surface */
			if (map->interpolation_type==DIRECT_INTERPOLATION)
			{
				if (!(data_field=get_unemap_package_scaled_offset_signal_value_at_time_field(
							unemap_package)))
				{
					map_define_scaled_offset_signal_at_time(unemap_package);
				data_field = get_unemap_package_scaled_offset_signal_value_at_time_field(
					unemap_package);
				}
			}
			/*BICUBIC_INTERPOLATION */
			else
			{
				fit_field=get_map_3d_package_fit_field(map_3d_package);
				data_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_is_read_only_with_fe_field,(void *)(fit_field),
					get_unemap_package_Computed_field_manager(unemap_package));
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_get_data_field_for_3d_map.  Invalid argument(s)");
		data_field = (struct Computed_field *)NULL;
	}
	LEAVE;
	
	return (data_field);
}/* map_get_data_field_for_3d_map */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int map_is_delaunay(struct Map *map)
/*******************************************************************************
LAST MODIFIED : 5 February 2002

DESCRIPTION :
Determines if a map is delaunay, baed upton projection type and if the default
torso is loaded
==============================================================================*/
{
	int return_code;
	struct Rig *rig;

	ENTER(map_is_deluney);
	if (map)
	{
		/* do we have a default torso loaded ? */
		if (map->interpolation_type==DIRECT_INTERPOLATION)
		{
			/* can only have DIRECT_INTERPOLATION for TORSO,THREED_PROJECTION*/
			/* these conditions should be ensured in open_map_dialog()*/
			if ((map->projection_type==THREED_PROJECTION)&&(map->rig_pointer)&&
				(rig=*(map->rig_pointer))&&(rig->current_region)&&
				(rig->current_region->type==TORSO))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"map_is_delaunay.  DIRECT_INTERPOLATION for wrong map type ");
				return_code=0;
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_is_delaunay.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_is_delaunay */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int map_electrodes_properties_changed(struct Map *map,
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
returns 1 if the <map_3d_package>s electrode properties have changed, 0 if they
haven't
==============================================================================*/
{
	char *electrode_glyph_name;
	int return_code;
	struct GT_object *electrode_glyph;

	ENTER(map_electrodes_properties_changed);
	electrode_glyph=(struct GT_object *)NULL;
	electrode_glyph_name=(char *)NULL;
	if (map&&map_3d_package)
	{
		return_code=0;
		if (electrode_glyph=get_map_3d_package_electrode_glyph(map_3d_package))
		{
			/* match the names to the marker types. Maybe store the marker types ??JW*/
			/*??JW maybe we should just store a flag to indicate that the electrode has*/
			/* changed, rather than storing all the electrodes_marker_size,electrode */
			/* option changed  etc at the package/map_info. If so, remove unemap/mapping.h*/
			/* from unemap_package.h cf map_settings_changed in update_map_from_dialog */
			/* in mapping_window.c */
			GET_NAME(GT_object)(electrode_glyph,&electrode_glyph_name);
			/*Glyph type has changed */
			if ((((!strcmp(electrode_glyph_name,"cross"))&&
				(map->electrodes_marker_type!=PLUS_ELECTRODE_MARKER))||
				((!strcmp(electrode_glyph_name,"sphere"))&&
					(map->electrodes_marker_type!=CIRCLE_ELECTRODE_MARKER))||
				((!strcmp(electrode_glyph_name,"diamond"))&&
					(map->electrodes_marker_type!=SQUARE_ELECTRODE_MARKER)))||
				/* electrode size changed*/
				(map->electrodes_marker_size!=
					get_map_3d_package_electrode_size(map_3d_package))||
				/* electrode option changed*/
				(map->electrodes_label_type!=
					get_map_3d_package_electrodes_label_type(map_3d_package))||
				/* colour_electrodes_with_signal flag has changed */
				(map->colour_electrodes_with_signal!=
					get_map_3d_package_colour_electrodes_with_signal(map_3d_package)))
			{
				return_code=1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_electrodes_properties_changed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_electrodes_properties_changed */
#endif /* UNEMAP_USE_3D */

#if defined (UNEMAP_USE_3D)
static int map_draw_map_electrodes(struct Unemap_package *unemap_package,
	struct Map *map)
/*******************************************************************************
LAST MODIFIED :  11 July 2000

DESCRIPTION :
Removes the map electrodes if they've changed, then redraws them for the current
region(s)
==============================================================================*/
{
	int display_all_regions,electrodes_properties_changed,return_code;
	struct GROUP(FE_node)	 *unrejected_node_group;
	struct Map_3d_package *map_3d_package;
	struct Map_drawing_information *drawing_information;
	struct Region_list_item *region_item;
	struct Region *region;
	struct Region *current_region;
	struct Rig *rig;

	ENTER(map_draw_map_electrodes);
	unrejected_node_group=(struct GROUP(FE_node) *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	region_item=(struct Region_list_item *)NULL;
	region=(struct Region *)NULL;
	current_region=(struct Region *)NULL;
	rig=(struct Rig *)NULL;
	if (unemap_package&&map&&(drawing_information=map->drawing_information)&&
		(map->rig_pointer)&&(rig= *(map->rig_pointer)))
	{
		return_code=1;
		display_all_regions=0;
		electrodes_properties_changed=0;
		current_region=get_Rig_current_region(rig);
		/*if current_region NULL, displayong all regions*/
		if (!current_region)
		{
			display_all_regions=1;
		}
		region_item=get_Rig_region_list(rig);
		/* work through all regions */
		while(region_item)
		{
			region=get_Region_list_item_region(region_item);
			unrejected_node_group=get_Region_unrejected_node_group(region);
			map_3d_package=get_Region_map_3d_package(region);
			if (map_3d_package)
			{
				electrodes_properties_changed=
					map_electrodes_properties_changed(map,map_3d_package);
			}
			else
			{
				electrodes_properties_changed=0;
			}
			/* remove undisplayed or changed electrodes */
			if (electrodes_properties_changed||(!display_all_regions&&
				(region!=current_region)))
			{
				map_remove_map_electrode_glyphs(drawing_information,
					unemap_package,map_3d_package,unrejected_node_group);
			}
			/* draw current electrodes */
			if (unrejected_node_group&&map_3d_package&&
				(unemap_package_rig_node_group_has_electrodes(unemap_package,
					unrejected_node_group))&&((region==current_region)||display_all_regions))
			{
				map_update_map_electrodes(unemap_package,region,map);
			}
			region_item=get_Region_list_item_next(region_item);
		}/* while(region_item)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,"map_draw_map_electrodes. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_draw_map_electrodes*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int draw_torso_arm_labels(struct Map_drawing_information *drawing_information,
	struct Region *region)/*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Creates a pair of arm labels, and adds to the scene.
==============================================================================*/
{
	int return_code;
	char **labels;
	char *left =(char *)NULL;
	char *right =(char *)NULL;
	FE_value min_x,max_x,min_y,max_y,min_z,max_z,x_offset,y_offset,z_offset;
	struct Graphical_material *graphical_material=(struct Graphical_material *)NULL;
	struct GT_glyph_set *glyph_set=(struct GT_glyph_set *)NULL;
	struct GT_object *glyph=(struct GT_object *)NULL;
	struct GT_object*graphics_object=(struct GT_object *)NULL;
	struct LIST(GT_object) *glyph_list=(struct LIST(GT_object) *)NULL;
	Triple *axis1_list=(Triple *)NULL;
	Triple *axis2_list=(Triple *)NULL;
	Triple *axis3_list=(Triple *)NULL;
	Triple *point_list=(Triple *)NULL;
	Triple *scale_list=(Triple *)NULL;

	ENTER(draw_torso_arm_labels);
	if (region&&drawing_information)
	{
		return_code=1;
		/*??JW should probably maintain a list of graphics_objects, and*/
		/*search in this list, as CMGUI does*/
		if ((!(graphics_object=
				get_map_drawing_information_torso_arm_labels(drawing_information)))&&
				get_node_group_position_min_max(
				get_Region_unrejected_node_group(region),
				get_Region_map_electrode_position_field(region),
				&min_x,&max_x,&min_y,&max_y,&min_z,&max_z))
		{
			if ((graphical_material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)
				("default",
				get_map_drawing_information_Graphical_material_manager(drawing_information)))&&
				(glyph_list=get_map_drawing_information_glyph_list(drawing_information))&&
				(glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",glyph_list))&&
				ALLOCATE(point_list,Triple,2)&&ALLOCATE(axis1_list,Triple,2)&&
				ALLOCATE(axis2_list,Triple,2)&&ALLOCATE(axis3_list,Triple,2)&&
				ALLOCATE(scale_list,Triple,2)&&
				ALLOCATE(labels,char *,2)&&ALLOCATE(left,char,5)&&ALLOCATE(right,char,6))
			{
				strcpy(left,"Left");
				strcpy(right,"Right");
				x_offset=max_x*1.4;
				y_offset=0;
				z_offset=max_z*1.05;
				labels[0]=left;
				labels[1]=right;
				(*point_list)[0]=x_offset;
				(*point_list)[1]=y_offset;
				(*point_list)[2]=z_offset;
				(*axis1_list)[0]=1;
				(*axis1_list)[1]=0.0;
				(*axis1_list)[2]=0.0;
				(*axis2_list)[0]=0.0;
				(*axis2_list)[1]=1;
				(*axis2_list)[2]=0.0;
				(*axis3_list)[0]=0.0;
				(*axis3_list)[1]=0.0;
				(*axis3_list)[2]=1;
				(*scale_list)[0]=1.0;
				(*scale_list)[1]=1.0;
				(*scale_list)[2]=1.0;

				(*(point_list+1))[0]=-x_offset;
				(*(point_list+1))[1]=y_offset;
				(*(point_list+1))[2]=z_offset;
				(*(axis1_list+1))[0]=1;
				(*(axis1_list+1))[1]=0.0;
				(*(axis1_list+1))[2]=0.0;
				(*(axis2_list+1))[0]=0.0;
				(*(axis2_list+1))[1]=1;
				(*(axis2_list+1))[2]=0.0;
				(*(axis3_list+1))[0]=0.0;
				(*(axis3_list+1))[1]=0.0;
				(*(axis3_list+1))[2]=1;
				(*(scale_list+1))[0]=1.0;
				(*(scale_list+1))[1]=1.0;
				(*(scale_list+1))[2]=1.0;
				if (!(glyph_set=CREATE(GT_glyph_set)(2,point_list,axis1_list,axis2_list,
					axis3_list,scale_list,glyph,labels,g_NO_DATA,(GTDATA *)NULL,
					/*object_name*/0,/*names*/(int *)NULL)))
				{
					DEALLOCATE(point_list);
					DEALLOCATE(axis1_list);
					DEALLOCATE(axis2_list);
					DEALLOCATE(axis3_list);
					DEALLOCATE(scale_list);
					DEALLOCATE(labels);
					DEALLOCATE(left);
					DEALLOCATE(right);
				}
			}
			if (glyph_set)
			{
				if (graphics_object=CREATE(GT_object)("arm_labels",g_GLYPH_SET,
					graphical_material))
				{
					if (GT_OBJECT_ADD(GT_glyph_set)(graphics_object,/*time*/0.0,glyph_set))
					{
						Scene_add_graphics_object(get_map_drawing_information_scene
							(drawing_information),graphics_object,0,
							graphics_object->name,/*fast_changing*/0);
						set_map_drawing_information_torso_arm_labels(drawing_information,
							graphics_object);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"draw_torso_arm_labels.  Could not add graphics object");
						DESTROY(GT_object)(&graphics_object);
						DESTROY(GT_glyph_set)(&glyph_set);
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"draw_torso_arm_labels.  Could not create glyph_set");
				graphics_object=(struct GT_object *)NULL;
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_torso_arm_labels.  Invalid argument(s)");
	}
	LEAVE;
	
	return (return_code);
}/* draw_torso_arm_labels */
#endif /* UNEMAP_USE_3D */

#if defined (UNEMAP_USE_3D)
static int map_remove_all_electrodes(struct Map *map)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
removes all the electrode glyphs used by the map, but DOESN'T  remove the
time computed fields used by the glyphs.
==============================================================================*/
{
	int return_code;
	struct Rig *rig;
	struct GROUP(FE_node) *unrejected_node_group;
	struct Region_list_item *region_item;
	struct Region *region;
	struct Map_3d_package *map_3d_package;
	struct Map_drawing_information *drawing_information;
	struct Unemap_package *unemap_package;

	ENTER(map_remove_all_electrodes);

	rig=(struct Rig *)NULL;
	unrejected_node_group=(struct GROUP(FE_node) *)NULL;
	region_item=(struct Region_list_item *)NULL;
	region=(struct Region *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	drawing_information	=(struct Map_drawing_information *)NULL;
	unemap_package=(struct Unemap_package *)NULL;
	if (map&&(map->rig_pointer)&&(rig= *(map->rig_pointer))&&
		(unemap_package=map->unemap_package)&&
		(drawing_information=map->drawing_information))
	{
		region_item=get_Rig_region_list(rig);
		return_code=1;
		while(region_item)
		{
			region=get_Region_list_item_region(region_item);
			unrejected_node_group=get_Region_unrejected_node_group(region);
			map_3d_package=get_Region_map_3d_package(region);
			if (unrejected_node_group&&map_3d_package&&
				(unemap_package_rig_node_group_has_electrodes(unemap_package,
					unrejected_node_group)))
			{
				map_remove_map_electrode_glyphs(drawing_information,
					unemap_package,map_3d_package,
					unrejected_node_group);
			}
			region_item=get_Region_list_item_next(region_item);
		}/* while(region_item)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,"map_remove_all_electrodes. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_remove_all_electrodes */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int merge_fit_field_template_element(struct FE_element *element,
	void *merge_fit_field_template_element_void)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Merges the template_element containing the fit field in
<merge_fit_field_template_element_void> with <element>
==============================================================================*/
{
	int i,return_code;
	struct FE_element *template_element, *unmanaged_element;
	struct CM_element_information cm;
	struct MANAGER(FE_element) *element_manager;
	struct merge_fit_field_template_element_data *merge_fit_field_template_element_data;
	struct FE_node *node;

	ENTER(merge_fit_field_template_element);
	merge_fit_field_template_element_data=
		(struct merge_fit_field_template_element_data *)NULL;
	if (element&&merge_fit_field_template_element_void&&
		(merge_fit_field_template_element_data=
		(struct merge_fit_field_template_element_data *)
		(merge_fit_field_template_element_void))&&
		(template_element=merge_fit_field_template_element_data->template_element)&&
		(element_manager=merge_fit_field_template_element_data->element_manager))
	{
		return_code=1;
		/* only operate on top level (quad elements, not lines */
		if (element->cm.type==CM_ELEMENT)
		{
			/* make template element use nodes from global element */
			i=0;
			while (return_code&&(i<4))
			{
				if (get_FE_element_node(element,i,&node))
				{
					return_code=set_FE_element_node(template_element,i,node);
				}
				else
				{
					return_code=0;
				}
				i++;
			}
			if (return_code)
			{
				cm.type = CM_ELEMENT;
				cm.number = 0;
				if (unmanaged_element=CREATE(FE_element)(&cm,element))
				{
					if (return_code=
						merge_FE_element(unmanaged_element,template_element))
					{
						/* copy the merged element back into the manager */
						return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_element,identifier)(
							element,unmanaged_element,element_manager);
					}
					DESTROY(FE_element)(&unmanaged_element);
				}
				else
				{
					return_code=0;
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,"merge_fit_field_template_element.  Failed");
			}
			merge_fit_field_template_element_data->count++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"merge_fit_field_template_element. "
			" Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* merge_fit_field_template_element */

static int set_up_fitted_potential_on_torso(struct GROUP(FE_element) *torso_group,
	struct FE_field *potential_field,struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager)
/*******************************************************************************
LAST MODIFIED :

DESCRIPTION :
Adds the <potential_field> to the elements in <torso_group>.
Does by creating a template element, adding <potential_field> to this,
and merging the template element with the elements in <torso_group>.
==============================================================================*/
{
	int number_of_nodes,number_of_derivatives,number_of_scale_factor_sets,
		*numbers_in_scale_factor_sets,k,l,m,return_code;
	int cubic_hermite_basis_type[4]=
	{
		2,CUBIC_HERMITE,0,CUBIC_HERMITE
	};
	int shape_type[3]=
	{
		LINE_SHAPE,0,0
	};
	struct CM_element_information element_identifier;
	struct FE_basis *cubic_hermite_basis;
	struct FE_element *template_element;
	struct FE_element_field_component **components;
	struct FE_element_shape *shape;
	struct merge_fit_field_template_element_data merge_fit_field_template_element_data;
	struct Standard_node_to_element_map **standard_node_map;
	void **scale_factor_set_identifiers;

	ENTER(set_up_fitted_potential_on_torso);
	return_code=0;
	cubic_hermite_basis=(struct FE_basis *)NULL;
	shape=(struct FE_element_shape *)NULL;
	template_element=(struct FE_element *)NULL;
	components=(struct FE_element_field_component **)NULL;
	standard_node_map=(struct Standard_node_to_element_map **)NULL;
	numbers_in_scale_factor_sets=(int *)NULL;
	if (torso_group&&potential_field)
	{
		if (cubic_hermite_basis=make_FE_basis(cubic_hermite_basis_type,basis_manager))
		{
			ACCESS(FE_basis)(cubic_hermite_basis);
			if (shape=CREATE(FE_element_shape)(2,shape_type))
			{
				ACCESS(FE_element_shape)(shape);
				/* define component */
				if (1==get_FE_field_number_of_components(potential_field))
				{
					element_identifier.type=CM_ELEMENT;
					element_identifier.number=1;
					number_of_scale_factor_sets=1;
					number_of_derivatives=3;
					if ((template_element=CREATE(FE_element)(&element_identifier,
						(struct FE_element *)NULL))&&
						(ALLOCATE(scale_factor_set_identifiers,void *,number_of_scale_factor_sets))
						&&(ALLOCATE(numbers_in_scale_factor_sets,int,number_of_scale_factor_sets)))
					{
						ACCESS(FE_element)(template_element);
						numbers_in_scale_factor_sets[0]=16;
						scale_factor_set_identifiers[0]=cubic_hermite_basis;
						number_of_nodes=4;
						if (set_FE_element_shape(template_element,shape)&&
							set_FE_element_node_scale_field_info(template_element,
							number_of_scale_factor_sets,scale_factor_set_identifiers,
							numbers_in_scale_factor_sets,number_of_nodes)&&
							(ALLOCATE(components,struct FE_element_field_component *,1)))
						{
							/* define the potential field over the template element */
							if (components[0]=CREATE(FE_element_field_component)(
								STANDARD_NODE_TO_ELEMENT_MAP,number_of_nodes,cubic_hermite_basis,
								(FE_element_field_component_modify)NULL))
							{
								/* create node map */
								standard_node_map=(components[0])->
									map.standard_node_based.node_to_element_maps;
								m=0;
								k=0;
								return_code=1;
								while (return_code&&(k<number_of_nodes))
								{
									if (*standard_node_map=CREATE(Standard_node_to_element_map)(
										/*node_index*/k,/*number_of_values*/4))
									{
										for (l=0;l<(1+number_of_derivatives);l++)
										{
											(*standard_node_map)->nodal_value_indices[l]=l;
											(*standard_node_map)->scale_factor_indices[l]=m;
											/* set scale_factors to 1 */
											template_element->information->scale_factors[
												(*standard_node_map)->scale_factor_indices[l]]=1.0;
											m++;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,"set_up_fitted_potential_on_torso "
											" could not create Standard_node_to_element_map");
										return_code=0;
									}
									standard_node_map++;
									k++;
								}
								if (return_code)
								{
									if (define_FE_field_at_element(template_element,
										potential_field,components))
									{
										/* add potential field to element */
										merge_fit_field_template_element_data.template_element=
											template_element;
										merge_fit_field_template_element_data.element_manager=
											element_manager;
										merge_fit_field_template_element_data.count=0;
										FOR_EACH_OBJECT_IN_GROUP(FE_element)(
											merge_fit_field_template_element,
											(void *)(&merge_fit_field_template_element_data),torso_group);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"set_up_fitted_potential_on_torso.  Could not define field");
										return_code=0;
									}
								}
								DESTROY(FE_element_field_component)(&(components[0]));
								DEALLOCATE(components);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_up_fitted_potential_on_torso.  Could not create component");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_up_fitted_potential_on_torso. "
								" Could not set element shape/scale factor info");
							return_code=0;
						}
						DEACCESS(FE_element)(&template_element);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_up_fitted_potential_on_torso.  Could not create template_element");
						return_code=0;
					}
					DEALLOCATE(scale_factor_set_identifiers);
					DEALLOCATE(numbers_in_scale_factor_sets);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_up_fitted_potential_on_torso.  Invalid potential field");
					return_code=0;
				}
				DEACCESS(FE_element_shape)(&shape);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_up_fitted_potential_on_torso.  Could not create shape");
				return_code=0;
			}
			DEACCESS(FE_basis)(&cubic_hermite_basis);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_up_fitted_potential_on_torso.  Could not create basis");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_up_fitted_potential_on_torso.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_up_fitted_potential_on_torso */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int define_fit_field_at_torso_element_nodes(
	struct FE_element *torso_element,
	void *define_fit_field_at_torso_element_nodes_data_void)
/*******************************************************************************
LAST MODIFIED : 8 November 2000

DESCRIPTION :
Get list of nodes in <torso_element>.
If <torso_element> is a CM_ELEMENT  AND has 4 nodes then
(
defines the fit field in <define_fit_field_at_torso_element_nodes_data> at
<torso_element>, adds  <torso_element> to quad_element_group in
<define_fit_field_at_torso_element_nodes_data>
)
===============================================================================*/
{
	int i,number_of_nodes,return_code;
	struct Define_FE_field_at_node_data define_FE_field_at_node_data;
	struct Define_fit_field_at_torso_element_nodes_data
		*define_fit_field_at_torso_element_nodes_data;
	struct FE_field *fit_field;
	struct GROUP(FE_element) *quad_element_group;
	struct FE_node **nodes_in_element;
	struct FE_node_field_creator *node_field_creator;
	struct MANAGER(FE_node) *node_manager;

	ENTER(define_fit_field_at_torso_element_nodes);
	return_code=0;
	fit_field=(struct FE_field *)NULL;
	quad_element_group=(struct GROUP(FE_element) *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	define_fit_field_at_torso_element_nodes_data=
		(struct Define_fit_field_at_torso_element_nodes_data *)NULL;
	if (torso_element&&(define_fit_field_at_torso_element_nodes_data_void)&&
		(define_fit_field_at_torso_element_nodes_data=
			(struct Define_fit_field_at_torso_element_nodes_data *)
			define_fit_field_at_torso_element_nodes_data_void)&&
		(quad_element_group=
			define_fit_field_at_torso_element_nodes_data->quad_element_group)&&
		(fit_field=define_fit_field_at_torso_element_nodes_data->fit_field)
		&&(node_manager=define_fit_field_at_torso_element_nodes_data->node_manager))
	{
		/* only deal with elements, not lines */
		if (CM_ELEMENT==torso_element->cm.type)
		{
			/* get all nodes in element */
			if (calculate_FE_element_field_nodes(torso_element,
				(struct FE_field *)NULL,&number_of_nodes,&nodes_in_element))
			{
				return_code=1;
				/* do nothing if not 4 nodes */
				if (4==number_of_nodes)
				{
					node_field_creator = CREATE(FE_node_field_creator)(
						/*number_of_components*/1);
					FE_node_field_creator_define_derivative(node_field_creator,
						/*component_number*/0, FE_NODAL_D_DS1);
					FE_node_field_creator_define_derivative(node_field_creator,
						/*component_number*/0, FE_NODAL_D_DS2);
					FE_node_field_creator_define_derivative(node_field_creator,
						/*component_number*/0, FE_NODAL_D2_DS1DS2);
					/* define the fit field at the element's nodes*/
					define_FE_field_at_node_data.field=fit_field;
					define_FE_field_at_node_data.node_field_creator=
						node_field_creator;
					define_FE_field_at_node_data.node_manager=node_manager;
					define_FE_field_at_node_data.time_version=
						(struct FE_time_version *)NULL;
					i=0;
					while ((i<number_of_nodes)&&
						(return_code=iterative_define_FE_field_at_node(nodes_in_element[i],
						(void *)(&define_FE_field_at_node_data))))
					{
						i++;
					}
					if (i>=number_of_nodes)
					{
						/* Add the element to the group */
						return_code=ADD_OBJECT_TO_GROUP(FE_element)(torso_element,
							quad_element_group);
					}
					for (i=0;i<number_of_nodes;i++)
					{
						DEACCESS(FE_node)(nodes_in_element+i);
					}
					DEALLOCATE(nodes_in_element);
					DESTROY(FE_node_field_creator)(&node_field_creator);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"define_fit_field_at_torso_element_nodes. "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* define_fit_field_at_torso_element_nodes*/
#endif /* defined (UNEMAP_USE_3D) */


#if defined (UNEMAP_USE_3D)
struct Vertices_data
{
	int current_element;
	float *vertices;
	struct FE_field *position_field;
};/* struct Vertices_data */

static int put_electrode_pos_into_vertices(struct FE_node *node,
	void *vertices_data_void)
/*******************************************************************************
LAST MODIFIED :17 November 2000

DESCRIPTION :
Fill in the vertices_data->vertices data with the nodal field values from <node>,
and vertices_data->position_field. Assumes that vertices_data->vertices has been
allocated, and that vertices_data->position_field has 3 FE_value components .
Updates vertices_data->current_element to keep track of how much of
vertices_data->vertices we've filled in.
==============================================================================*/
{
	int return_code;
	FE_value value;
	struct FE_field *position_field;
	struct FE_field_component component;
	struct Vertices_data *vertices_data;

	ENTER(put_electrode_pos_into_vertices);
	if (node&&vertices_data_void&&(vertices_data=
		(struct Vertices_data *)vertices_data_void)
		&&(position_field=vertices_data->position_field))
	{
		component.field=position_field;
		component.number=0;
		return_code=get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
			/*time*/0,&value);
		if (return_code)
		{
			vertices_data->vertices[vertices_data->current_element]=(float)(value);
			vertices_data->current_element++;
			component.number=1;
			return_code=get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
				/*time*/0,&value);
			if (return_code)
			{
				vertices_data->vertices[vertices_data->current_element]=(float)(value);
				vertices_data->current_element++;
				component.number=2;
				return_code=get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
					/*time*/0,&value);
				if (return_code)
				{
					vertices_data->vertices[vertices_data->current_element]=(float)(value);
					vertices_data->current_element++;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			" put_electrode_pos_into_vertices. invalid arguments ");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}/* put_electrode_pos_into_vertices */

static struct FE_element *make_delaunay_template_element(
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager,struct FE_field *data_field,
	struct FE_field *coordinate_field, struct FE_field *gain_field,
	struct FE_field *offset_field)
/*******************************************************************************
LAST MODIFIED :17 November 2000

DESCRIPTION :
Makes and accesses and returns the template element for the delaunay
map. <data_field> and <coordinate_field> are defined at the element.

Therefore must deaccess the returned element outside this function
==============================================================================*/
{
	int number_of_nodes,*numbers_in_scale_factor_sets,
		number_of_scale_factor_sets,m,k,p,success;
	int basis_type[4]=
	{
		2,LINEAR_SIMPLEX,1,LINEAR_SIMPLEX
	};
	int shape_type[3]=
	{
		SIMPLEX_SHAPE,1,SIMPLEX_SHAPE
	};

	struct CM_element_information element_identifier;
	struct Coordinate_system *data_coordinate_system,*coordinate_system;
	struct FE_basis *basis;
	struct FE_element_shape *shape;
	struct FE_element *template_element;
	struct FE_element_field_component **data_components,**coord_components;
	struct Standard_node_to_element_map **standard_node_map;
	void **scale_factor_set_identifiers;

	ENTER(make_delaunay_template_element);
	basis=(struct FE_basis *)NULL;
	shape=(struct FE_element_shape *)NULL;
	template_element=(struct FE_element *)NULL;
	data_components=(struct FE_element_field_component **)NULL;
	coord_components=(struct FE_element_field_component **)NULL;
	standard_node_map=(struct Standard_node_to_element_map **)NULL;
	scale_factor_set_identifiers=(void **)NULL;
	coordinate_system=(struct Coordinate_system *)NULL;
	data_coordinate_system=(struct Coordinate_system *)NULL;

	if (basis_manager&&element_manager&&data_field&&coordinate_field)
	{
		if (basis=make_FE_basis(basis_type,basis_manager))
		{
			ACCESS(FE_basis)(basis);
			if (shape=CREATE(FE_element_shape)(2,shape_type))
			{
				ACCESS(FE_element_shape)(shape);
				data_coordinate_system=get_FE_field_coordinate_system(data_field);
				coordinate_system=get_FE_field_coordinate_system(coordinate_field);
				/* check fields have correct number of components and coordinate_system */
				/* delaunay can only be done on electrodes with RC coord sys, arranged*/
				/* approximately in a cylinder */
				if ((1==get_FE_field_number_of_components(data_field))&&
					(data_coordinate_system->type==NOT_APPLICABLE)&&
					(3==get_FE_field_number_of_components(coordinate_field))&&
					(coordinate_system->type==RECTANGULAR_CARTESIAN))
				{
					element_identifier.type=CM_ELEMENT;
					element_identifier.number=1;
					number_of_scale_factor_sets=1;
					if ((template_element=CREATE(FE_element)(&element_identifier,
						(struct FE_element *)NULL))&&
						(ALLOCATE(scale_factor_set_identifiers,void *,number_of_scale_factor_sets))
						&&(ALLOCATE(numbers_in_scale_factor_sets,int,number_of_scale_factor_sets)))
					{
						ACCESS(FE_element)(template_element);
						numbers_in_scale_factor_sets[0]=3;
						scale_factor_set_identifiers[0]=basis;
						number_of_nodes=3;
						if (set_FE_element_shape(template_element,shape)&&
							set_FE_element_node_scale_field_info(template_element,
								number_of_scale_factor_sets,scale_factor_set_identifiers,
								numbers_in_scale_factor_sets,number_of_nodes)&&
							(ALLOCATE(coord_components,struct FE_element_field_component *,3))&&
							(ALLOCATE(data_components,struct FE_element_field_component *,1)))
						{
							/* do things for data field*/
							if (data_components[0]=CREATE(FE_element_field_component)(
								STANDARD_NODE_TO_ELEMENT_MAP,number_of_nodes,basis,
								(FE_element_field_component_modify)NULL))
							{
								/* create node map */
								standard_node_map=(data_components[0])->
									map.standard_node_based.node_to_element_maps;
								m=0;
								k=0;
								success=1;
								while (success&&(k<number_of_nodes))
								{
									if (*standard_node_map=CREATE(Standard_node_to_element_map)(
										/*node_index*/k,/*number_of_values*/1))
									{
										(*standard_node_map)->nodal_value_indices[0]=0;
										(*standard_node_map)->scale_factor_indices[0]=m;
										/* scale_factors set when actual element created from template */
										m++;
									}
									else
									{
										display_message(ERROR_MESSAGE,"make_delaunay_template_element could not create "
											"Standard_node_to_element_map");
										success=0;
									}
									standard_node_map++;
									k++;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"make_delaunay_template_element. Could not create data_component");
								success=0;
							}
							/* do things for coordinate field*/
							/* 3 components of coordinate field */
							for(p=0;p<3;p++)
							{
								if (coord_components[p]=CREATE(FE_element_field_component)(
									STANDARD_NODE_TO_ELEMENT_MAP,number_of_nodes,basis,
									(FE_element_field_component_modify)NULL))
								{
									/* create node map */
									standard_node_map=(coord_components[p])->
										map.standard_node_based.node_to_element_maps;
									m=0;
									k=0;
									success=1;
									while (success&&(k<number_of_nodes))
									{
										if (*standard_node_map=CREATE(Standard_node_to_element_map)(
											/*node_index*/k,/*number_of_values*/1))
										{
											(*standard_node_map)->nodal_value_indices[0]=0;
											(*standard_node_map)->scale_factor_indices[0]=m;
											/* scale_factors set when actual element created from template */
											m++;
										}
										else
										{
											display_message(ERROR_MESSAGE,"make_delaunay_template_element could not create "
												"Standard_node_to_element_map");
											success=0;
										}
										standard_node_map++;
										k++;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"make_delaunay_template_element. Could not create coord_component");
									success=0;
								}
							}
							if (success)
							{
								if (!(define_FE_field_at_element(template_element,data_field,data_components)&&
									define_FE_field_at_element(template_element,coordinate_field,coord_components)&&
									define_FE_field_at_element(template_element,gain_field,data_components)&&
									define_FE_field_at_element(template_element,offset_field,data_components)))
								{
									display_message(ERROR_MESSAGE,
										"make_delaunay_template_element.  Could not define field");
									success=0;
								}
							}
							if (data_components)
							{
								DESTROY(FE_element_field_component)(&(data_components[0]));
								DEALLOCATE(data_components);
							}
							if (coord_components)
							{
								DESTROY(FE_element_field_component)(&(coord_components[0]));
								DESTROY(FE_element_field_component)(&(coord_components[1]));
								DESTROY(FE_element_field_component)(&(coord_components[2]));
								DEALLOCATE(coord_components);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"make_delaunay_template_element. "
								" Could not set element shape/scale factor info");
							success=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"make_delaunay_template_element.  Could not create template_element");
						success=0;
					}
					DEALLOCATE(scale_factor_set_identifiers);
					DEALLOCATE(numbers_in_scale_factor_sets);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"make_delaunay_template_element.  Invalid  field");
					success=0;
				}
				DEACCESS(FE_element_shape)(&shape);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"make_delaunay_template_element.  Could not create shape");
				success=0;
			}
			DEACCESS(FE_basis)(&basis);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"make_delaunay_template_element.  Could not create basis");
			success=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			" make_delaunay_template_element. invalid arguments ");
		success=0;
	}
	if (success==0)
	{
		DEACCESS(FE_element)(&template_element);
		template_element=(struct FE_element *)NULL;
	}
	LEAVE;
	
	return (template_element);
}/* make_delaunay_template_element */

static int make_delaunay_node_and_element_group(struct GROUP(FE_node) *source_nodes,
	struct FE_field *electrode_postion_field,struct FE_field *delaunay_signal_field,
	struct FE_field *channel_gain_field,struct FE_field *channel_offset_field,
	struct Unemap_package *unemap_package,struct Region *region)
/*******************************************************************************
LAST MODIFIED :17 November 2000

DESCRIPTION :
Given the <source_nodes>, performs delaunay triangularisation on them,
and produces elements (and an element group) from these triangles.
Produces same name node and data groups for graphics. Defines the
<electrode_postion_field> and a data field at the elements.

Requires (and checks) that <electrode_postion_field>  and the data fields
are already defined at the nodes.

Stores node and element groups in region <map_3d_package>
==============================================================================*/
{
	char delaunay_group_name[]="delaunay";
	int i,j,number_of_triangles,number_of_vertices,return_code,*triangles,
		*vertex_number;
	float *vertices;
	struct CM_element_information element_identifier;
	struct FE_element *template_element,*element;
	struct FE_node *triangle_nodes[3],*vertex_node;
	struct FE_node_order_info *node_order_info;
	struct GROUP(FE_element) *delaunay_element_group;
	struct GROUP(FE_node) *delaunay_node_group;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_basis) *basis_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct Vertices_data vertices_data;
	struct Map_3d_package *map_3d_package;

	ENTER(make_delaunay_node_and_element_group);
	vertices=(float *)NULL;
	triangles =(int *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	vertex_node=(struct FE_node *)NULL;
	delaunay_element_group=(struct GROUP(FE_element) *)NULL;
	delaunay_node_group=(struct GROUP(FE_node) *)NULL;
	template_element=(struct FE_element *)NULL;
	element=(struct FE_element *)NULL;
	node_order_info=(struct FE_node_order_info *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	if (source_nodes&&electrode_postion_field&&delaunay_signal_field&&unemap_package
		&&region&&(element_group_manager=
			get_unemap_package_element_group_manager(unemap_package))&&
		(node_group_manager=
			get_unemap_package_node_group_manager(unemap_package))&&
		(data_group_manager=
			get_unemap_package_data_group_manager(unemap_package))&&
		(basis_manager=get_unemap_package_basis_manager(unemap_package))&&
		(node_manager=get_unemap_package_node_manager(unemap_package))&&
		(element_manager=get_unemap_package_element_manager(unemap_package)))
	{
		/* check fields defined at all nodes*/
		if (!(FIRST_OBJECT_IN_GROUP_THAT(FE_node)(FE_node_field_is_not_defined,
			(void *)delaunay_signal_field,source_nodes))&&
			!(FIRST_OBJECT_IN_GROUP_THAT(FE_node)(FE_node_field_is_not_defined,
				(void *)electrode_postion_field,source_nodes))&&
			!(FIRST_OBJECT_IN_GROUP_THAT(FE_node)(FE_node_field_is_not_defined,
			(void *)channel_gain_field,source_nodes))&&
			!(FIRST_OBJECT_IN_GROUP_THAT(FE_node)(FE_node_field_is_not_defined,
				(void *)channel_offset_field,source_nodes))&&
			/*create template element*/
			(template_element=make_delaunay_template_element(basis_manager,element_manager,
				delaunay_signal_field,electrode_postion_field,channel_gain_field,
				channel_offset_field)))
		{
			element_identifier.type=CM_ELEMENT;
			element_identifier.number=1;
			number_of_vertices=(NUMBER_IN_GROUP(FE_node)(source_nodes));
			if (ALLOCATE(vertices,float,3*number_of_vertices))
			{
				/* put the nodal position field data into the vertex array*/
				vertices_data.current_element=0;
				vertices_data.vertices=vertices;
				vertices_data.position_field=electrode_postion_field;
				if ((return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(put_electrode_pos_into_vertices,
					(void *)&vertices_data,source_nodes))&&
					/*perform the delaunay triangularisation*/
					(return_code=cylinder_delaunay(number_of_vertices,vertices,
						&number_of_triangles,&triangles)))
				{
					return_code=1;
					/* create same name node,element,data groups so can do graphics */
					delaunay_node_group=make_node_and_element_and_data_groups(
						node_group_manager,node_manager,element_manager,element_group_manager,
						data_group_manager,delaunay_group_name);
					MANAGED_GROUP_BEGIN_CACHE(FE_node)(delaunay_node_group);
					delaunay_element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
						(delaunay_group_name,element_group_manager);
					MANAGED_GROUP_BEGIN_CACHE(FE_element)(delaunay_element_group);
					if (delaunay_element_group&&delaunay_node_group)
					{
						return_code=1;
						if (return_code)
						{
							/* make a node_order_info, so can match nodes to vertices  */
							if ((node_order_info=CREATE(FE_node_order_info)(0))&&
								(FOR_EACH_OBJECT_IN_GROUP(FE_node)(fill_FE_node_order_info,
									(void *)node_order_info,source_nodes)))
							{
								/* find nodes from triangle info, put in node group*/
								/* when full, delaunay_node_group should match source_nodes group */
								/* but a future mapping may not, and want own same name data and element */
								/* groups for graphics */
								vertex_number=triangles;
								i=0;
								while((i<number_of_triangles)&&(return_code))
								{
									/* do all 3 nodes of triangle*/
									j=0;
									while((j<3)&&(return_code))
									{
										vertex_node=get_FE_node_order_info_node(node_order_info,
											*vertex_number);
										if (vertex_node)
										{
											/* store triangle nodes to make element*/
											triangle_nodes[j]=vertex_node;
											if ((!IS_OBJECT_IN_GROUP(FE_node)(vertex_node,
												delaunay_node_group))&&
												(!ADD_OBJECT_TO_GROUP(FE_node)(vertex_node,
													delaunay_node_group)))
											{
												display_message(ERROR_MESSAGE,
													"make_delaunay_node_and_element_group."
													" failed to add node to group");
												REMOVE_OBJECT_FROM_GROUP(FE_node)(vertex_node,
													delaunay_node_group);
												return_code=0;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"make_delaunay_node_and_element_group."
												" vertex node not found");
											return_code=0;
										}
										vertex_number++;
										j++;
									}	/* while((j<3)&&(return_code))	*/
									/* find first free element identifier  */
									while (element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,
										identifier)(&element_identifier,element_manager))
									{
										element_identifier.number++;
									}
									/* create element with template, add nodes */
									if (element=CREATE(FE_element)(&element_identifier,
										template_element))
									{
										if (set_FE_element_node(element,0,triangle_nodes[0])&&
											set_FE_element_node(element,1,triangle_nodes[1])&&
											set_FE_element_node(element,2,triangle_nodes[2]))
										{
											/*set the scale factors */
											FE_element_set_scale_factor_for_nodal_value(element,triangle_nodes[0],
												delaunay_signal_field,0/*component_number*/,FE_NODAL_VALUE,1.0);
											FE_element_set_scale_factor_for_nodal_value(element,triangle_nodes[1],
												delaunay_signal_field,0/*component_number*/,FE_NODAL_VALUE,1.0);
											FE_element_set_scale_factor_for_nodal_value(element,triangle_nodes[2],
												delaunay_signal_field,0/*component_number*/,FE_NODAL_VALUE,1.0);
											/* add element to manager*/
											if (ADD_OBJECT_TO_MANAGER(FE_element)(element,element_manager))
											{
												/* add element to group */
												if (!ADD_OBJECT_TO_GROUP(FE_element)(element,
													delaunay_element_group))
												{
													display_message(ERROR_MESSAGE,
														"make_delaunay_node_and_element_group.  "
														"Could not add element to element_group");
													/* remove element from manager to destroy it */
													REMOVE_OBJECT_FROM_MANAGER(FE_element)(element,
														element_manager);
													return_code=0;
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"make_delaunay_node_and_element_group.  "
													"Could not add element to element_manager");
												DESTROY(FE_element)(&element);
												return_code=0;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"make_delaunay_node_and_element_group. "
												"Could not set element nodes");
											DESTROY(FE_element)(&element);
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											" make_delaunay_node_and_element_group. "
											"Could not create element");
										return_code=0;
									}
									i++;
								}	/* while((i<number_of_triangles)&&(return_code)) */
								/* store the created groups */
								if (return_code)
								{
									/* create map_3d_package, if necessary */
									map_3d_package=get_Region_map_3d_package(region);
									if (!map_3d_package)
									{
										map_3d_package=CREATE(Map_3d_package)(
											get_unemap_package_FE_field_manager(unemap_package),
											get_unemap_package_element_group_manager(unemap_package),
											get_unemap_package_data_manager(unemap_package),
											get_unemap_package_data_group_manager(unemap_package),
											get_unemap_package_node_manager(unemap_package),
											get_unemap_package_node_group_manager(unemap_package),
											get_unemap_package_element_manager(unemap_package),
											get_unemap_package_Computed_field_manager(unemap_package));
										set_Region_map_3d_package(region,map_3d_package);
									}
									set_map_3d_package_delaunay_torso_node_group(map_3d_package,
										delaunay_node_group);
									set_map_3d_package_delaunay_torso_element_group(map_3d_package,
										delaunay_element_group);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE," make_delaunay_node_and_element_group. "
									"Could not make node_order_info");
								return_code=0;
							}

						}
						else
						{
							display_message(ERROR_MESSAGE," make_delaunay_node_and_element_group. "
								"Could not add groups to managers");

						}
					}
					else
					{
						display_message(ERROR_MESSAGE," make_delaunay_node_and_element_group. "
							"Could not make groups");

					}
					MANAGED_GROUP_END_CACHE(FE_node)(delaunay_node_group);
					MANAGED_GROUP_END_CACHE(FE_element)(delaunay_element_group);
				}
				else
				{
					display_message(ERROR_MESSAGE," make_delaunay_node_and_element_group. "
						"Could not do delaunay triangularisation ");
					return_code=0;
				}
				if (node_order_info)
				{
					DESTROY(FE_node_order_info)(&node_order_info);
				}
				if (template_element)
				{
					DEACCESS(FE_element)(&template_element);
				}
				if (vertices)
				{
					DEALLOCATE(vertices);
				}
				if (triangles)
				{
					DEALLOCATE(triangles);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					" make_delaunay_node_and_element_group. Out of memory");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				" make_delaunay_node_and_element_group. failed to make template element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			" make_delaunay_node_and_element_group. Can't find element group");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* make_delaunay_node_and_element_group */

struct Set_delaunay_signal_data
/*******************************************************************************
LAST MODIFIED :5 December 2000

DESCRIPTION :
Used by iterative_set_delaunay_signal_nodal_value
==============================================================================*/
{
	struct FE_field *channel_gain_field,*channel_offset_field,
		*delaunay_signal_field,*signal_field;
	FE_value time;
	struct MANAGER(FE_node) *node_manager;
}; /* Set_delaunay_signal_data */

#if defined (OLD_CODE)
static int iterative_set_delaunay_signal_nodal_value(struct FE_node *node,
	void *set_delaunay_signal_data_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2000

DESCRIPTION : Given time, channel_gain_field, channel_offset_field signal_field,
and delaunay_signal_field in <set_delaunay_signal_data_void>,
sets the <node> delaunay_signal_field. This is scaled and offset.
NOTES:
Assumes signal_field,delaunay_signal_field defined at the node.
Assumes signal_field is a time based FE_value array field.
??JW This function is an interim meaure until we are able to set array fields
at elements (or any field other than fe_vale fields at elements).
cf map_set_electrode_colour_from_time
===============================================================================*/
{
	enum Value_type value_type;
	int return_code;
	FE_value fe_value,gain,offset,time;
	short short_value;
	struct FE_field_component component;
	struct FE_field *channel_gain_field,*channel_offset_field,*signal_field,
		*delaunay_signal_field;
	struct FE_node *node_copy;
	struct MANAGER(FE_node) *node_manager;
	struct Set_delaunay_signal_data *set_delaunay_signal_data;

	ENTER(iterative_set_delaunay_signal_nodal_value);
	node_copy=(struct FE_node *)NULL;
	delaunay_signal_field=(struct FE_field *)NULL;
	signal_field=(struct FE_field *)NULL;
	channel_gain_field=(struct FE_field *)NULL;
	channel_offset_field=(struct FE_field *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	set_delaunay_signal_data=(struct Set_delaunay_signal_data *)NULL;
	if (node&&set_delaunay_signal_data_void&&(set_delaunay_signal_data=
		(struct Set_delaunay_signal_data *)set_delaunay_signal_data_void))
	{
		time=set_delaunay_signal_data->time;
		signal_field=set_delaunay_signal_data->signal_field;
		delaunay_signal_field=
			set_delaunay_signal_data->delaunay_signal_field;
		channel_gain_field=set_delaunay_signal_data->channel_gain_field;
		channel_offset_field=set_delaunay_signal_data->channel_offset_field;
		node_manager=set_delaunay_signal_data->node_manager;
		if (FE_field_is_defined_at_node(signal_field,node)&&
			FE_field_is_defined_at_node(delaunay_signal_field,node)&&
			FE_field_is_defined_at_node(channel_gain_field,node)&&
			FE_field_is_defined_at_node(channel_offset_field,node))
		{
			/* get the signal value at the given time*/
			value_type=get_FE_field_value_type(signal_field);
			component.number=0;
			component.field=signal_field;
			switch (value_type)
			{
				case FE_VALUE_VALUE:
				{
					return_code=get_FE_nodal_FE_value_value(node,
						&component,0,FE_NODAL_VALUE,time,&fe_value);
				} break;
				case SHORT_VALUE:
				{
					return_code=get_FE_nodal_short_value_at_time(node,
						&component,0,FE_NODAL_VALUE,time,&short_value);
					fe_value=short_value;
				} break;
				default :
				{
					display_message(ERROR_MESSAGE,
						"iterative_set_delaunay_signal_nodal_value. "
						" Incorrect signal field value type");
					return_code=0;
				} break;
			}
			if (return_code)
			{
				/* scale and offset */
				component.field=channel_gain_field;
				get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
					/*time*/0,&gain);
				component.field=channel_offset_field;
				get_FE_nodal_FE_value_value(node,&component,0,FE_NODAL_VALUE,
					/*time*/0,&offset);
				fe_value+=offset;
				fe_value*=gain;
				/* set the signal_value_at_time field from this*/
				component.field=delaunay_signal_field;
				/* create a node to work with */
				node_copy=CREATE(FE_node)(0,(struct FE_node *)NULL);
				/* copy it from the manager */
				if (MANAGER_COPY_WITH_IDENTIFIER(FE_node,cm_node_identifier)
					(node_copy,node))
				{
					return_code=set_FE_nodal_FE_value_value(node_copy,&component,0,FE_NODAL_VALUE,
						fe_value);
					MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
						(node,node_copy,node_manager);
				}
				else
				{
					display_message(ERROR_MESSAGE,"iterative_set_delaunay_signal_nodal_value."
						" MANAGER_COPY_WITH_IDENTIFIER failed ");
					return_code=0;
				}
				/* destroy the working copy */
				DESTROY(FE_node)(&node_copy);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"iterative_set_delaunay_signal_nodal_value."
				" fields not defined at nodes");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"iterative_set_delaunay_signal_nodal_value."
			" Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /*iterative_set_delaunay_signal_nodal_value */
#endif /* defined (OLD_CODE) */

#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int make_and_set_delaunay(struct Region *region,
	struct Unemap_package *unemap_package,FE_value time,
	int nodes_rejected_or_accepted)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
Makes and/or sets the nodal values in the delaunay node and element groups
==============================================================================*/
{
	int return_code;
	struct FE_field *electrode_postion_field,*delaunay_signal_field,
		*channel_gain_field, *channel_offset_field;
#if defined (OLD_CODE)
	struct FE_node *node;
	struct GROUP(FE_node) *rig_node_group;
	struct MANAGER(FE_node) *node_manager;
#endif /* defined (OLD_CODE) */
	struct GROUP(FE_node) *unrejected_nodes;
	struct Map_3d_package *map_3d_package;
#if defined (OLD_CODE)
	struct Set_delaunay_signal_data set_delaunay_signal_data;
#endif /* defined (OLD_CODE) */

	ENTER(make_and_set_delaunay);
	USE_PARAMETER(time);
	electrode_postion_field=(struct FE_field *)NULL;
	delaunay_signal_field=(struct FE_field *)NULL;
	channel_gain_field=(struct FE_field *)NULL;
	channel_offset_field=(struct FE_field *)NULL;
#if defined (OLD_CODE)
	rig_node_group=(struct GROUP(FE_node) *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	node=(struct FE_node *)NULL;
#endif /* defined (OLD_CODE) */
	unrejected_nodes=(struct GROUP(FE_node) *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	if (region&&unemap_package&&
		(electrode_postion_field=get_Region_electrode_position_field(region)))
	{
		return_code=1;
		if (!(delaunay_signal_field=get_unemap_package_signal_field(unemap_package))
			|| !(channel_gain_field=get_unemap_package_channel_gain_field(unemap_package))
			|| !(channel_offset_field=get_unemap_package_channel_offset_field(unemap_package)))
		{
			display_message(ERROR_MESSAGE,"make_and_set_delaunay.  "
						"signal field, gain field or offset field not found ");
			return_code=0;
		}
		/* check if haven't made delaunay groups yet, or for newly rejected/accpted */
		/*electrodes to see if remake delaunay groups */
		map_3d_package=get_Region_map_3d_package(region);
		if (nodes_rejected_or_accepted||(!map_3d_package)||(map_3d_package&&
			(!get_map_3d_package_delaunay_torso_node_group(map_3d_package))))
		{

			if (map_3d_package)
			{
				/*this will do a deaccess*/
				set_Region_map_3d_package(region,(struct Map_3d_package *)NULL);
			}
			/*  make delaunay groups */
			if ((!map_3d_package)||(map_3d_package&&
				(!get_map_3d_package_delaunay_torso_node_group(map_3d_package))))
			{
				if (return_code&&(unrejected_nodes=get_Region_unrejected_node_group(region)))
				{
					return_code=make_delaunay_node_and_element_group(unrejected_nodes,
						electrode_postion_field,delaunay_signal_field,
						channel_gain_field,channel_offset_field,unemap_package,region);
				}
				else
				{
					display_message(ERROR_MESSAGE,"make_and_set_delaunay"
							" failed to make delaunay node and element groups");
				}
			}/* if ((!map_3d_package)||(map_3d_package&& */
		}/* (nodes_rejected_or_accepted) */
#if defined (OLD_CODE)
		/* map_3d_package may have changed*/
		map_3d_package=get_Region_map_3d_package(region);
		if (map_3d_package&&(delaunay_torso_node_group=
			get_map_3d_package_delaunay_torso_node_group(map_3d_package)))
		{
			MANAGER_BEGIN_CACHE(FE_node)(node_manager);
			/*set the delaunay_signal_field at the nodes from time */
			set_delaunay_signal_data.delaunay_signal_field=
				delaunay_signal_field;
			set_delaunay_signal_data.signal_field=
				get_unemap_package_signal_field(unemap_package);
			set_delaunay_signal_data.channel_gain_field=
				get_unemap_package_channel_gain_field(unemap_package);
			set_delaunay_signal_data.channel_offset_field=
				get_unemap_package_channel_offset_field(unemap_package);
			set_delaunay_signal_data.time=time;
			set_delaunay_signal_data.node_manager=node_manager;
			return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)
				(iterative_set_delaunay_signal_nodal_value,
					(void *)(&set_delaunay_signal_data),delaunay_torso_node_group);
			MANAGER_END_CACHE(FE_node)(node_manager);
		}
		else
		{
			display_message(ERROR_MESSAGE,"make_and_set_delaunay."
				" no group to set");
			return_code=0;
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_and_set_delaunay."
			" Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* make_and_set_delaunay */
#endif /*defined (UNEMAP_USE_3D) */

/*
Global functions
----------------
*/
#if defined (UNEMAP_USE_3D)
/* proportion of the contour that is blacked. 1000=1, 100=0.1  */
#define CONTOUR_PROPORTION 100
int map_draw_contours(struct Map *map,	struct Spectrum *spectrum,
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Draws (or erases) the map contours
==============================================================================*/
{
	int default_torso_loaded,delaunay_map,number_of_constant_contours,
		number_of_variable_contours,return_code;
	struct Computed_field *data_field;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Map_3d_package *map_3d_package;
	struct Scene *scene;
	struct Rig *rig;
	struct GROUP(FE_node) *unrejected_node_group;
	struct Region_list_item *region_item;
	struct Region *region;

	ENTER(map_draw_contours);
	map_3d_package=(struct Map_3d_package *)NULL;
	spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
	scene=(struct Scene *)NULL;
	rig=(struct Rig *)NULL;
	unrejected_node_group=(struct GROUP(FE_node) *)NULL;
	region_item=(struct Region_list_item *)NULL;
	region=(struct Region *)NULL;
	/*data_field can be NULL*/
	if (map&&&package&&spectrum&&(map->rig_pointer)&&(rig= *(map->rig_pointer))&&
		(map->drawing_information)&&(spectrum_manager=
			get_map_drawing_information_spectrum_manager(map->drawing_information))&&
		(scene=get_map_drawing_information_scene(map->drawing_information)))
	{
		/* Show the contours */
		if ((map->contours_option==SHOW_CONTOURS)&&
			(map->interpolation_type!=NO_INTERPOLATION))
		{
			if (map->contour_thickness==CONSTANT_THICKNESS)
			{
				number_of_constant_contours=map->number_of_contours;
				number_of_variable_contours=0;
			}
			else
				/* map->contour_thickness==VARIABLE_THICKNESS */
			{
				number_of_constant_contours=0;
				/*-1 as unemap treats number of contours differently from the cmgui spectrum */
				/* it's a fencepost thing */
				number_of_variable_contours=map->number_of_contours-1;
			}
		}
		else
		{
			number_of_variable_contours=0;
			number_of_constant_contours=0;
		}
		/* draw/remove VARIABLE_THICKNESS contours*/
		if (get_unemap_package_torso_group_name(package))
		{
			default_torso_loaded=1;
		}
		else
		{
			default_torso_loaded=0;
		}
		Spectrum_overlay_contours(spectrum_manager,spectrum,
			number_of_variable_contours,CONTOUR_PROPORTION);
		region_item=get_Rig_region_list(rig);
		while(region_item)
		{
			region=get_Region_list_item_region(region_item);
			unrejected_node_group=get_Region_unrejected_node_group(region);
			map_3d_package=get_Region_map_3d_package(region);
			if (unrejected_node_group&&
				(unemap_package_rig_node_group_has_electrodes(package,unrejected_node_group))&&
				(map_3d_package = get_Region_map_3d_package(region)))
			{
				data_field = map_get_data_field_for_3d_map(map, map_3d_package);
				delaunay_map = map_is_delaunay(map);
				/* draw/remove CONSTANT_THICKNESS contours */
				map_draw_constant_thickness_contours(scene,map->drawing_information,
					data_field,number_of_constant_contours,map->contour_minimum,
					map->contour_maximum,region,default_torso_loaded,delaunay_map);
			}
			region_item=get_Region_list_item_next(region_item);
		}/* while(region_item)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_draw_contours. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}/* map_draw_contours */
#endif /* defined (UNEMAP_USE_3D) */

struct Map *create_Map(enum Map_type *map_type,enum Colour_option colour_option,
	enum Contours_option contours_option,
	enum Electrodes_label_type electrodes_label_type,
	enum Fibres_option fibres_option,enum Landmarks_option landmarks_option,
	enum Extrema_option extrema_option,int maintain_aspect_ratio,
	int print_spectrum,enum Projection_type projection_type,
	enum Contour_thickness contour_thickness,struct Rig **rig_pointer,
	int *event_number_address,int *potential_time_address,int *datum_address,
	int *start_search_interval_address,int *end_search_interval_address,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface,struct Unemap_package *unemap_package,
	struct Electrical_imaging_event **eimaging_event_list,
	enum Signal_analysis_mode *analysis_mode)
/*******************************************************************************
LAST MODIFIED : 1 May 2002

DESCRIPTION :
This function allocates memory for a map and initializes the fields to the
specified values.  It returns a pointer to the created map if successful and
NULL if not successful.
==============================================================================*/
{
	char *electrodes_marker_type,*finite_element_interpolation,
		*fixed_automatic_range,*draw_on_update;
	int membrane_smoothing_ten_thous,plate_bending_smoothing_ten_tho;
#define XmNfiniteElementInterpolation "finiteElementInterpolation"
#define XmCFiniteElementInterpolation "FiniteElementInterpolation"
#define XmNfiniteElementMeshRows "finiteElementMeshRows"
#define XmCFiniteElementMeshRows "FiniteElementMeshRows"
#define XmNfiniteElementMeshRows "finiteElementMeshRows"
#define XmCFiniteElementMeshRows "FiniteElementMeshRows"
#define XmNfiniteElementMeshColumns "finiteElementMeshColumns"
#define XmCFiniteElementMeshColumns "FiniteElementMeshColumns"
#define XmNmembraneSmoothingTenThous "membraneSmoothingTenThous"
#define XmCMembraneSmoothingTenThous "MembraneSmoothingTenThous"
#define XmNplateBendingSmoothingTenThous "plateBendingSmoothingTenThous"
#define XmCPlateBendingSmoothingTenThous "PlateBendingSmoothingTenThous"
#define XmNelectrodeMarker "electrodeMarker"
#define XmCElectrodeMarker "ElectrodeMarker"
#define XmNelectrodeSize "electrodeSize"
#define XmCElectrodeSize "ElectrodeSize"
#define XmNmapRange "mapRange"
#define XmCMapRange "MapRange"
#define XmNmapRangeMinimum "mapRangeMinimum"
#define XmCMapRangeMinimum "MapRangeMinimum"
#define XmNmapRangeMaximum "mapRangeMaximum"
#define XmCMapRangeMaximum "MapRangeMaximum"
#define XmNreDrawMapOnManualTimeUpdate "reDrawMapOnManualTimeUpdate"
#define XmCReDrawMapOnManualTimeUpdate "ReDrawMapOnManualTimeUpdate"
	static XtResource
		resources_1[]=
		{
			{
				XmNfiniteElementMeshRows,
				XmCFiniteElementMeshRows,
				XmRInt,
				sizeof(int),
				XtOffsetOf(Map_settings,finite_element_mesh_rows),
				XmRString,
				"3"
			},
			{
				XmNfiniteElementMeshColumns,
				XmCFiniteElementMeshColumns,
				XmRInt,
				sizeof(int),
				XtOffsetOf(Map_settings,finite_element_mesh_columns),
				XmRString,
				"4"
			}
		},
		resources_2[]=
		{
			{
				XmNmembraneSmoothingTenThous,
				XmCMembraneSmoothingTenThous,
				XmRInt,
				sizeof(int),
				0,
				XmRString,
				"100"
			}
		},
		resources_3[]=
		{
			{
				XmNplateBendingSmoothingTenThous,
				XmCPlateBendingSmoothingTenThous,
				XmRInt,
				sizeof(int),
				0,
				XmRString,
				"10"
			}
		},
		resources_4[]=
		{
			{
				XmNfiniteElementInterpolation,
				XmCFiniteElementInterpolation,
				XmRString,
				sizeof(char *),
				0,
				XmRString,
				"bicubic"
			}
		},
		resources_5[]=
		{
			{
				XmNelectrodeMarker,
				XmCElectrodeMarker,
				XmRString,
				sizeof(char *),
				0,
				XmRString,
				"plus"
			}
		},
		resources_6[]=
		{
			{
				XmNelectrodeSize,
				XmCElectrodeSize,
				XmRInt,
				sizeof(int),
				0,
				XmRString,
				"2"
			}
		},
		resources_7[]=
		{
			{
				XmNmapRange,
				XmCMapRange,
				XmRString,
				sizeof(char *),
				0,
				XmRString,
				"automatic"
			}
		},
		resources_8[]=
		{
			{
				XmNmapRangeMinimum,
				XmCMapRangeMinimum,
				XmRFloat,
				sizeof(float),
				0,
				XmRString,
				"1"
			}
		},
		resources_9[]=
		{
			{
				XmNmapRangeMaximum,
				XmCMapRangeMaximum,
				XmRFloat,
				sizeof(float),
				0,
				XmRString,
				"0"
			}
		},
		resources_10[]=
		{
			{
				XmNreDrawMapOnManualTimeUpdate,
				XmCReDrawMapOnManualTimeUpdate,
				XmRString,
				sizeof(char *),
				0,
				XmRString,
				"false"
			}
		};
	struct Map *map;

	ENTER(create_Map);
	USE_PARAMETER(unemap_package);
	/* check arguments */
	if (user_interface&&map_drawing_information
#if defined (UNEMAP_USE_3D)
		&&unemap_package
#endif /* defined (UNEMAP_USE_3D) */
			)
	{
		/* allocate memory */
		if (ALLOCATE(map,struct Map,1))
		{
			map->start_time=0;
			map->end_time=0;
			map->number_of_sub_maps=0;
			map->number_of_frames=0;
			map->sub_map_number=0;
			map->sub_map=(struct Sub_map **)NULL;
			map->contour_x_spacing=0;
			map->contour_y_spacing=0;
			map->number_of_region_rows=1;
			map->number_of_region_columns=1;
			map->number_of_gouraud_triangles=0;
			map->triangle_electrode_indices=(int *)NULL;
			map->electrode_tex_x=(int *)NULL;
			map->electrode_tex_y=(int *)NULL;
			map->electrode_rgbs=(unsigned char *)NULL;
			map->texture_image=(unsigned char *)NULL;
			/* assign fields */
			map->analysis_mode=analysis_mode;
			map->first_eimaging_event=eimaging_event_list;
			map->type=map_type;
			map->event_number=event_number_address;
			map->potential_time=potential_time_address;
			map->datum=datum_address;
			map->start_search_interval=start_search_interval_address;
			map->end_search_interval=end_search_interval_address;
			map->contours_option=contours_option;
			map->colour_option=colour_option;
			map->electrodes_label_type=electrodes_label_type;
			map->fibres_option=fibres_option;
			map->landmarks_option=landmarks_option;
			map->extrema_option=extrema_option;
			map->maintain_aspect_ratio=maintain_aspect_ratio;
			map->print_spectrum=print_spectrum;
			map->projection_type=projection_type;
			map->contour_thickness=contour_thickness;
			map->undecided_accepted=0;
			map->rig_pointer=rig_pointer;
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),map,
				resources_1,XtNumber(resources_1),NULL);
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&membrane_smoothing_ten_thous,resources_2,XtNumber(resources_2),NULL);
			map->membrane_smoothing=(float)membrane_smoothing_ten_thous/10000;
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&plate_bending_smoothing_ten_tho,resources_3,XtNumber(resources_3),
				NULL);
			map->plate_bending_smoothing=(float)plate_bending_smoothing_ten_tho/10000;
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&finite_element_interpolation,resources_4,XtNumber(resources_4),
				NULL);
			if (fuzzy_string_compare(finite_element_interpolation,"bicubic"))
			{
				map->interpolation_type=BICUBIC_INTERPOLATION;
			}
			else
			{
				if (fuzzy_string_compare(finite_element_interpolation,"direct"))
				{
					map->interpolation_type=DIRECT_INTERPOLATION;
				}
				else
				{
					map->interpolation_type=NO_INTERPOLATION;
				}
			}
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&electrodes_marker_type,resources_5,XtNumber(resources_5),
				NULL);
			if (fuzzy_string_compare(electrodes_marker_type,"square"))
			{
				map->electrodes_marker_type=SQUARE_ELECTRODE_MARKER;
			}
			else
			{
				if (fuzzy_string_compare(electrodes_marker_type,"circle"))
				{
					map->electrodes_marker_type=CIRCLE_ELECTRODE_MARKER;
				}
				else
				{
					map->electrodes_marker_type=PLUS_ELECTRODE_MARKER;
				}
			}
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&(map->electrodes_marker_size),resources_6,XtNumber(resources_6),
				NULL);
			if (map->electrodes_marker_size<1)
			{
				map->electrodes_marker_size=1;
			}
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&fixed_automatic_range,resources_7,XtNumber(resources_7),
				NULL);
			if (fuzzy_string_compare(fixed_automatic_range,"automatic"))
			{
				map->fixed_range=0;
			}
			else
			{
				map->fixed_range=1;
			}
			map->range_changed=0;
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&(map->minimum_value),resources_8,XtNumber(resources_8),
				NULL);
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&(map->maximum_value),resources_9,XtNumber(resources_9),
				NULL);
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&draw_on_update,resources_10,XtNumber(resources_10),NULL);
			if (fuzzy_string_compare(draw_on_update,"true"))
			{
				map->draw_map_on_manual_time_update=1;
			}
			else
			{
				map->draw_map_on_manual_time_update=0;
			}
			map->colour_electrodes_with_signal=1;
			/*??? calculate from rig ? */
			map->number_of_electrodes=0;
			map->electrodes=(struct Device **)NULL;
			map->electrode_drawn=(char *)NULL;
			map->draw_region_number=(int *)NULL;
			map->number_of_drawn_regions=0;
			map->number_of_auxiliary=0;
			map->auxiliary=(struct Device **)NULL;
			map->auxiliary_x=(int *)NULL;
			map->auxiliary_y=(int *)NULL;
			map->contour_minimum=map->minimum_value;
			map->contour_maximum=map->maximum_value;
			map->number_of_contours=2;
			map->activation_front= -1;
			map->print=0;
			map->drawing_information=map_drawing_information;
#if defined (UNEMAP_USE_3D)
			map->unemap_package = unemap_package;
			/* register for any spectrum changes */
#else
			map->unemap_package = (struct Unemap_package *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
		}
		else
		{
			display_message(ERROR_MESSAGE,"create_Map.  Could not allocate map");
			DEALLOCATE(map);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Map.  Invalid argument(s)");
		map=(struct Map *)NULL;
	}
	LEAVE;

	return (map);
} /* create_Map */

int Map_flush_cache(struct Map *map)
/*******************************************************************************
LAST MODIFIED : 1 March 2001

DESCRIPTION :
Clear the cached information stored in the map
==============================================================================*/
{
	int return_code;

	ENTER(Map_flush_cache);
	return_code=1;
	if (map)
	{
		DEALLOCATE(map->triangle_electrode_indices);
		map->number_of_gouraud_triangles=0;
		DEALLOCATE(map->electrode_tex_x);
		DEALLOCATE(map->electrode_tex_y);
		DEALLOCATE(map->texture_image);
		DEALLOCATE(map->electrode_rgbs);
		DEALLOCATE(map->electrodes);
		map->number_of_electrodes=0;
		DEALLOCATE(map->electrode_drawn);
		map->number_of_auxiliary=0;
		DEALLOCATE(map->auxiliary);
		DEALLOCATE(map->auxiliary_x);
		DEALLOCATE(map->auxiliary_y);
		DEALLOCATE(map->draw_region_number);
		map->number_of_drawn_regions=0;
		DEALLOCATE(map->electrode_drawn);
		map_destroy_Sub_maps(map);
		map->number_of_sub_maps=0;
	}
	LEAVE;
	
	return (return_code);
} /* Map_flush_cache */

int destroy_Map(struct Map **map)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
This function deallocates the memory asociated with fields of <**map> (except
the <rig_pointer>  and Unemap_package fields  ) ,
deallocates the memory for <**map> and sets <*map> to NULL.
==============================================================================*/
{
	int return_code;
	ENTER(destroy_Map);
	return_code=1;
	if (map&&(*map))
	{
		Map_flush_cache(*map);
		DEALLOCATE(*map);
	}
	LEAVE;

	return (return_code);
} /* destroy_Map */

static int map_2d_delaunay(struct Map *map)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
given <map>, do a delaunay triangluarisation of map->electrodes.
Assumes the map is of type TORSO, and hence is cylindrical, and centred about
the z axis.
The results are stored as map->number_of_gouraud_triangles, the number of delaunay
triagles formed by the electrodes, and map->triangle_electrode_indices,
and array of length 3*number_of_gouraud_triangles containing indices into
map->electrodes of pointing to the triangle's electrodes.
No cutting of the cylinder for 2d projection is done. This happens after 2d
projection is performed, in  draw_2d_construct_2d_gouraud_image_map
*******************************************************************************/
{
	char undecided_accepted;
	float *vertex,*vertices;
	int found,i,index,j,*map_electrode_triangles,number_of_electrodes,
		number_of_triangles,number_of_accepted_triangles,number_of_vertices,
		return_code,*triangles,*triangle_vertex,triangle_v0_index,
		triangle_v1_index,triangle_v2_index;
	struct Device **accepted_triangle_electrodes,**electrode,**triangle_electrodes,
		**triangle_electrode,*device;
	struct Device_description *description;

	ENTER(map_2d_delaunay);
	device=(struct Device *)NULL;
	electrode=(struct Device **)NULL;
	accepted_triangle_electrodes=(struct Device **)NULL;
	triangle_electrodes=(struct Device **)NULL;
	triangle_electrode=(struct Device **)NULL;
	description=(struct Device_description *)NULL;
	vertices=(float *)NULL;
	vertex=(float *)NULL;
	triangles =(int *)NULL;
	triangle_vertex =(int *)NULL;
	if (map)
	{
		undecided_accepted=map->undecided_accepted;
		/* clear out any old stuff */
		map->number_of_gouraud_triangles=0;
		if (map->triangle_electrode_indices)
		{
			DEALLOCATE(map->triangle_electrode_indices);
		}
		return_code=1;
		number_of_vertices=0;
		electrode=map->electrodes;
		number_of_electrodes=0;
		/*count the number of vertices = number of unrejected electrodes in map */
		while(number_of_electrodes<map->number_of_electrodes)
		{
			/*if we've got an unrejected electrode */
			if (((*electrode)->signal->status==ACCEPTED)||
				(((*electrode)->signal->status==UNDECIDED)&&undecided_accepted))
			{
				number_of_vertices++;
			}
			electrode++;
			number_of_electrodes++;
		}/*while(number_of_devices<rig->number_of_devices)*/
		/*allocate space for x,y,z vertices and the triangle_electrodes */
		if (ALLOCATE(vertices,float,number_of_vertices*3)&&
			ALLOCATE(triangle_electrodes,struct Device *,number_of_vertices))
		{
			/*for unrejected electrodes,store the vertex values & electrode/device*/
			electrode=map->electrodes;
			number_of_electrodes=0;
			vertex=vertices;
			triangle_electrode=triangle_electrodes;
			while(number_of_electrodes<map->number_of_electrodes)
			{
				description=(*electrode)->description;
				/*if we've got an unrejected electrode */
				if (((*electrode)->signal->status==ACCEPTED)||
					(((*electrode)->signal->status==UNDECIDED)&&undecided_accepted))
				{
					*vertex=(description->properties).electrode.position.x;
					vertex++;
					*vertex=(description->properties).electrode.position.y;
					vertex++;
					*vertex=(description->properties).electrode.position.z;
					vertex++;
					*triangle_electrode=*electrode;
					triangle_electrode++;
				}
				electrode++;
				number_of_electrodes++;
			}/*while(number_of_devices<rig->number_of_devices)*/
			/*perform deluaney triangularization */
			if (return_code=cylinder_delaunay(number_of_vertices,vertices,
				&number_of_triangles,&triangles))
			{
				/*accepted_triangles contains the Device's of the vertices triangles */
				if (ALLOCATE(accepted_triangle_electrodes,struct Device *,
					number_of_triangles*3))
				{
					triangle_vertex=triangles;
					number_of_accepted_triangles=0;
					for(i=0;i<number_of_triangles;i++)
					{
						triangle_v0_index=*triangle_vertex;
						triangle_vertex++;
						triangle_v1_index=*triangle_vertex;
						triangle_vertex++;
						triangle_v2_index=*triangle_vertex;
						triangle_vertex++;
						/*store Device's triangle electrodes*/
						number_of_accepted_triangles++;
						index=(number_of_accepted_triangles-1)*3;
						/*vertex 0,1,2*/
						accepted_triangle_electrodes[index]=
							triangle_electrodes[triangle_v0_index];
						accepted_triangle_electrodes[index+1]=
							triangle_electrodes[triangle_v1_index];
						accepted_triangle_electrodes[index+2]=
							triangle_electrodes[triangle_v2_index];
					}
					/*map_electrode_triangles contains index in map->electrodes array of */
					/*triangle's vertices/electrodes */
					if (ALLOCATE(map_electrode_triangles,int,number_of_triangles*3))
					{
						return_code=1;
						/*match device->number to position in map->electrodes array */
						i=0;
						while((i<number_of_accepted_triangles*3)&&(return_code))
						{
							device=accepted_triangle_electrodes[i];
							j=0;
							found=0;
							electrode=map->electrodes;
							while((j<map->number_of_electrodes)&&(!found))
							{
								if ((*electrode)==device)
								{
									map_electrode_triangles[i]=j;
									found=1;
								}
								else
								{
									j++;
									electrode++;
								}
							}/* while((j<map->number_of_electrodes)&&(!found)) */
							if (!found)
							{
								display_message(ERROR_MESSAGE,
									"map_2d_delaunay. unmatched triangle vertex");
								return_code=0;
							}
							i++;
						}/* while((i<number_of_accepted_triangles*3)&&(return_code)) */
						if (return_code)
						{
							map->number_of_gouraud_triangles=number_of_accepted_triangles;
							map->triangle_electrode_indices=map_electrode_triangles;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"map_2d_delaunay. Out of memory for map_electrode_triangles");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"map_2d_delaunay. Out of memory for accepted_triangles");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"map_2d_delaunay. cylinder_delaunay failed");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"map_2d_delaunay. Out of memory for vertices/electrode_numbers");
			return_code=0;
		}
		DEALLOCATE(vertices);
		DEALLOCATE(triangle_electrodes);
		DEALLOCATE(triangles);
		DEALLOCATE(accepted_triangle_electrodes);
	}
	else
	{
		display_message(ERROR_MESSAGE,"map_2d_delaunay. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_2d_delaunay */

int update_colour_map_unemap_original(struct Map *map,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 20 September 2001

DESCRIPTION :
Updates the colour map being used for map.
Called by (see also) update_colour_map_unemap.
???DB.  <drawing> added because of read only colour maps.
==============================================================================*/
{
	Colormap colour_map;
	Display *display;
	float background_pixel_value,blue,boundary_pixel_value,contour_maximum,
		contour_minimum,f_approx,green,max_f,maximum_value,min_f,minimum_value,
		*pixel_value,range_f,red,theta;
	int cell_number,cell_range,drawing_height,drawing_width,end_cell,i,j,
		number_of_contours,number_of_spectrum_colours,return_code,start_cell,
		update_pixel[MAX_SPECTRUM_COLOURS],update_pixel_assignment,x_pixel,y_pixel;
	Pixel background_pixel,boundary_pixel,*spectrum_pixels;
	struct Map_drawing_information *drawing_information;
	struct Map_frame *map_frame;
	unsigned short blue_short,green_short,red_short;
	XColor colour,*spectrum_rgb;
	XImage *map_image;
	struct Spectrum *spectrum=(struct Spectrum *)NULL;
	struct Spectrum *spectrum_copy=(struct Spectrum *)NULL;
	struct Spectrum *spectrum_to_be_modified_copy=(struct Spectrum *)NULL;
	struct MANAGER(Spectrum) *spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
	struct Sub_map *sub_map;

	ENTER(update_colour_map_unemap_original);
#if !defined (UNEMAP_USE_3D)
	USE_PARAMETER(spectrum);
	USE_PARAMETER(spectrum_to_be_modified_copy);
	USE_PARAMETER(spectrum_manager);
#endif/* defined (UNEMAP_USE_3D)*/
	if (map&&(map->type)&&(drawing_information=map->drawing_information)&&
		(drawing_information->user_interface)&&drawing)
	{
		return_code=1;
		display=User_interface_get_display(drawing_information->user_interface);
		minimum_value=map->minimum_value;
		maximum_value=map->maximum_value;
		contour_minimum=map->contour_minimum;
		contour_maximum=map->contour_maximum;
		number_of_spectrum_colours=drawing_information->number_of_spectrum_colours;
		colour_map=drawing_information->colour_map;
		boundary_pixel=drawing_information->boundary_colour;
		spectrum_pixels=drawing_information->spectrum_colours;
		spectrum_rgb=drawing_information->spectrum_rgb;
		/* for read only colour maps */
		update_pixel_assignment=0;
		for (i=0;i<MAX_SPECTRUM_COLOURS;i++)
		{
			update_pixel[i]=0;
		}
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
		if ((SHOW_COLOUR==map->colour_option)&&
			!((SINGLE_ACTIVATION== *(map->type))&&(0<=map->activation_front)&&
			(map->activation_front<number_of_spectrum_colours)))
		{
			/* copy the spectrum (leave unmanaged) and generate the mapping window */
			/* colour bar from this. Do this as need to remove  */
			/* fix_minimum, fix_maximum from the spectrum as these prevent the mapping */
			/* window colour bar from working correctly  */
			spectrum=map->drawing_information->spectrum;
			if (spectrum_copy=CREATE(Spectrum)("spectrum_copy"))
			{
				MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
					(spectrum_copy,spectrum);
				/* must now remove the fix_minimum, fix_maximum from spectrum */
				Spectrum_clear_all_fixed_flags(spectrum_copy);
				Spectrum_set_minimum_and_maximum(spectrum_copy,0.0,6.0);
			}
			for (i=0;i<=start_cell;i++)
			{
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				theta = 0.0;
				spectrum_value_to_rgb(spectrum_copy,
					/* number_of_data_components */1, &theta,&red,&green,&blue);
				red_short=(unsigned short)(65535*red);
				green_short=(unsigned short)(65535*green);
				blue_short=(unsigned short)(65535*blue);
				if (!(spectrum_pixels[i])||(spectrum_rgb[i].red!=red_short)||
					(spectrum_rgb[i].green!=green_short)||
					(spectrum_rgb[i].blue!=blue_short))
				{
					spectrum_rgb[i].red=red_short;
					spectrum_rgb[i].green=green_short;
					spectrum_rgb[i].blue=blue_short;
					update_pixel[i]=1;
				}
			}
			for (i=start_cell+1;i<end_cell;i++)
			{
				theta=(float)(i-start_cell)/(float)(cell_range)*6;
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				spectrum_value_to_rgb(spectrum_copy,
					/* number_of_data_components */1, &theta,&red,&green,&blue);
				red_short=(unsigned short)(65535*red);
				green_short=(unsigned short)(65535*green);
				blue_short=(unsigned short)(65535*blue);
				if (!(spectrum_pixels[i])||(spectrum_rgb[i].red!=red_short)||
					(spectrum_rgb[i].green!=green_short)||
					(spectrum_rgb[i].blue!=blue_short))
				{
					spectrum_rgb[i].red=red_short;
					spectrum_rgb[i].green=green_short;
					spectrum_rgb[i].blue=blue_short;
					update_pixel[i]=1;
				}
			}
			for (i=end_cell;i<number_of_spectrum_colours;i++)
			{
				theta = 6.0;
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				spectrum_value_to_rgb(spectrum_copy,
					/* number_of_data_components */1, &theta,&red,&green,&blue);
				red_short=(unsigned short)(65535*red);
				green_short=(unsigned short)(65535*green);
				blue_short=(unsigned short)(65535*blue);
				if (!(spectrum_pixels[i])||(spectrum_rgb[i].red!=red_short)||
					(spectrum_rgb[i].green!=green_short)||
					(spectrum_rgb[i].blue!=blue_short))
				{
					spectrum_rgb[i].red=red_short;
					spectrum_rgb[i].green=green_short;
					spectrum_rgb[i].blue=blue_short;
					update_pixel[i]=1;
				}
			}
			/* hide the map boundary */
			if (drawing_information->read_only_colour_map)
			{
				if (drawing_information->boundary_colour!=
					drawing_information->background_drawing_colour)
				{
					update_pixel_assignment=1;
					drawing_information->boundary_colour=
						drawing_information->background_drawing_colour;
				}
			}
			else
			{
				if ((Pixel)NULL!=boundary_pixel)
				{
					colour.pixel=drawing_information->background_drawing_colour;
					XQueryColor(display,colour_map,&colour);
					colour.pixel=boundary_pixel;
					colour.flags=DoRed|DoGreen|DoBlue;
					XStoreColor(display,colour_map,&colour);
				}
			}
		}
		else
		{
			/* use background drawing colour for the whole spectrum */
			colour.pixel=drawing_information->background_drawing_colour;
			XQueryColor(display,colour_map,&colour);
			for (i=0;i<number_of_spectrum_colours;i++)
			{
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				if (!(spectrum_rgb[i].pixel)||(spectrum_rgb[i].red!=colour.red)||
					(spectrum_rgb[i].green!=colour.green)||
					(spectrum_rgb[i].blue!=colour.blue))
				{
					spectrum_rgb[i].red=colour.red;
					spectrum_rgb[i].blue=colour.blue;
					spectrum_rgb[i].green=colour.green;
					update_pixel[i]=1;
				}
			}
			/* show the map boundary */
			if (drawing_information->read_only_colour_map)
			{
				if (drawing_information->boundary_colour!=
					drawing_information->contour_colour)
				{
					update_pixel_assignment=1;
					drawing_information->boundary_colour=
						drawing_information->contour_colour;
				}
			}
			else
			{
				if ((Pixel)NULL!=boundary_pixel)
				{
					colour.pixel=drawing_information->contour_colour;
					XQueryColor(display,colour_map,&colour);
					colour.pixel=boundary_pixel;
					colour.flags=DoRed|DoGreen|DoBlue;
					XStoreColor(display,colour_map,&colour);
				}
			}
			if ((SINGLE_ACTIVATION== *(map->type))&&(0<=map->activation_front)&&
				(map->activation_front<number_of_spectrum_colours))
			{
				/* show the activation front */
				colour.pixel=drawing_information->contour_colour;
				XQueryColor(display,colour_map,&colour);
				i=map->activation_front;
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				if (!(spectrum_pixels[i])||(spectrum_rgb[i].red!=colour.red)||
					(spectrum_rgb[i].green!=colour.green)||
					(spectrum_rgb[i].blue!=colour.blue))
				{
					spectrum_rgb[i].red=colour.red;
					spectrum_rgb[i].blue=colour.blue;
					spectrum_rgb[i].green=colour.green;
					update_pixel[i]=1;
				}
			}
		}
		/* adjust the computer colour map for contours */
		if (SHOW_CONTOURS==map->contours_option)
		{
			if ((VARIABLE_THICKNESS==map->contour_thickness)
				/* ||!(map_frame->pixel_values)*/)
			{
				colour.pixel=drawing_information->contour_colour;
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
					if (!(spectrum_pixels[cell_number])||
						(spectrum_rgb[cell_number].red!=colour.red)||
						(spectrum_rgb[cell_number].green!=colour.green)||
						(spectrum_rgb[cell_number].blue!=colour.blue))
					{
						spectrum_rgb[cell_number].red=colour.red;
						spectrum_rgb[cell_number].blue=colour.blue;
						spectrum_rgb[cell_number].green=colour.green;
						update_pixel[cell_number]=1;
					}
				}
			}
		}
		if (drawing_information->read_only_colour_map)
		{
			/*loop through all the sub_maps */
			for(j=0;j<map->number_of_sub_maps;j++)
			{
				sub_map=map->sub_map[j];
				map_frame=&(sub_map->frame);
				map_image=map_frame->image;
				for (i=0;i<number_of_spectrum_colours;i++)
				{
					if (update_pixel[i])
					{
						colour.pixel=spectrum_rgb[i].pixel;
						colour.flags=spectrum_rgb[i].flags;
						colour.red=spectrum_rgb[i].red;
						colour.blue=spectrum_rgb[i].blue;
						colour.green=spectrum_rgb[i].green;
						XAllocColor(display,colour_map,&colour);
						if (spectrum_pixels[i]!=colour.pixel)
						{
							update_pixel_assignment=1;
							spectrum_pixels[i]=colour.pixel;
						}
					}
				}
				if (update_pixel_assignment&&map_image)
				{
					min_f=map->minimum_value;
					max_f=map->maximum_value;
					background_pixel_value=sub_map->minimum_value;
					boundary_pixel_value=sub_map->maximum_value;
					pixel_value=map_frame->pixel_values;
					background_pixel=drawing_information->background_drawing_colour;
					boundary_pixel=drawing_information->boundary_colour;
					drawing_height=sub_map->height;
					drawing_width=sub_map->width;
					/* calculate range of values */
					range_f=max_f-min_f;
					if (range_f<=0)
					{
						range_f=1;
					}
					for (y_pixel=0;y_pixel<drawing_height;y_pixel++)
					{
						for (x_pixel=0;x_pixel<drawing_width;x_pixel++)
						{
							f_approx= *pixel_value;
							if (f_approx>=background_pixel_value)
							{
								if (f_approx<=boundary_pixel_value)
								{
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
								}
								else
								{
									XPutPixel(map_image,x_pixel,y_pixel,
										boundary_pixel);
								}
							}
							else
							{
								XPutPixel(map_image,x_pixel,y_pixel,
									background_pixel);
							}
							pixel_value++;
						}/*for (x_pixel=0*/
					}/* for (y_pixel=0;y_ */
				}/* if (update_pixel_assignment&&map_image)*/
			}/* for(j=0;j<map->number_of_sub_maps;j++) */
		}
		else
		{
			if (0<number_of_spectrum_colours)
			{
				XStoreColors(display,colour_map,spectrum_rgb,
					number_of_spectrum_colours);
			}
		}
		/* no longer need the spectrum copy */
		DESTROY(Spectrum)(&spectrum_copy);
#if defined (UNEMAP_USE_3D)
		spectrum=map->drawing_information->spectrum;
		if (spectrum_manager=
			get_map_drawing_information_spectrum_manager(drawing_information))
		{
			if (IS_MANAGED(Spectrum)(spectrum,spectrum_manager))
			{
				if (spectrum_to_be_modified_copy=CREATE(Spectrum)
					("spectrum_modify_temp"))
				{
					MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
						(spectrum_to_be_modified_copy,spectrum);
					/*Ensure spectrum is set correctly */
					Spectrum_set_minimum_and_maximum(spectrum_to_be_modified_copy,
						map->minimum_value,map->maximum_value);

					MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
						spectrum_to_be_modified_copy,spectrum_manager);
					DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						" update_colour_map_unemap_original. Could not create spectrum copy.");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					" update_colour_map_unemap_original. Spectrum is not in manager!");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				" update_colour_map_unemap_original. Spectrum_manager not present");
		}
#else /* defined (UNEMAP_USE_3D) */
		/* ensure spectrum is set correctly */
		Spectrum_set_minimum_and_maximum(map->drawing_information->spectrum,
			map->minimum_value,map->maximum_value);
#endif /* defined (UNEMAP_USE_3D) */
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,"update_colour_map_unemap_original.  "
			"Missing map");
	}
	LEAVE;

	return (return_code);
} /* update_colour_map_unemap_original */

int update_colour_map_unemap(struct Map *map,struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 20 September  2001

DESCRIPTION :
Calls update_colour_map_unemap_original, the relevant function twice,
as for BLUE_WHITE_RED_SPECTRUM need to do (something in)
update_colour_map_unemap_original again, as  fix_minimum, fix_maximum cause problems
??JW Fix this bug!
==============================================================================*/
{
	int return_code;
	struct Map_drawing_information *drawing_information;

	return_code=update_colour_map_unemap_original(map,drawing);
	/*??JW this is a problem! for BLUE_WHITE_RED_SPECTRUM need to do*/
	/* (something in) update_colour_map_unemap_original again, as */
	/* fix_minimum, fix_maximum cause problems */
	{
		drawing_information=map-> drawing_information;
		if (BLUE_WHITE_RED_SPECTRUM == Spectrum_get_simple_type(
			drawing_information->spectrum))
		{
			return_code=update_colour_map_unemap_original(map,drawing);
		}
	}
	LEAVE;
	
	return (return_code);
}
#if defined (UNEMAP_USE_3D)
int map_remove_torso_arm_labels(struct Map_drawing_information *drawing_information)/*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Removes the torso arms labels from the scene.
??JW perhaps should have a scene and scene_viewer in each Map_3d_package,
and do Scene_remove_graphics_object in the DESTROY Map_3d_package
==============================================================================*/
{
	int return_code;
	ENTER(map_remove_torso_arm_labels);
	if (drawing_information)
	{
		if (drawing_information->torso_arm_labels)
		{
			Scene_remove_graphics_object(drawing_information->scene,
				drawing_information->torso_arm_labels);
			DEACCESS(GT_object)(&(drawing_information->torso_arm_labels));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"map_remove_torso_arm_labels. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* map_remove_torso_arm_labels */
#endif /* defined (UNEMAP_USE_3D) */

static int get_triangle_line_end_vertex_num_from_start_num(int start_vertex_num)
/*******************************************************************************
LAST MODIFIED 14 November 2001

DESCRIPTION :
Given <start_vertex_num>, the start vertex number of an edge of a triangle,
returns end_vertex_num, the corresponding end vertex number of the same edge.
<start_vertex_num> most be from 0->2.
Sets end_vertex_num according to:
start_vertex_num 0 : end_vertex_num 1 (triangle edge 0)
start_vertex_num 1 end_vertex_num 2 (triangle edge 1)
start_vertex_num 2 end_vertex_num 0 (triangle edge 2)
Returns -1 on error.
A helper function for get_triangle_scan_line_intersection,
draw_2d_construct_2d_gouraud_image_map,
get_tri_scan_line_intersection_rgb_triple
*******************************************************************************/
{
	int end_vertex_num;

	ENTER(get_triangle_line_end_vertex_num_from_start_num);
	if ((start_vertex_num>-1)&&(start_vertex_num<3))
	{
		if (start_vertex_num<2)
		{
			end_vertex_num=start_vertex_num+1;
		}
		else
		{
			end_vertex_num=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_triangle_line_end_vertex_num_from_start_num. Invalid arguments");
		end_vertex_num=-1;
	}
	LEAVE;

	return (end_vertex_num);
}/*get_triangle_line_end_vertex_num_from_start_num */

#if defined (UNEMAP_USE_3D)
static int get_tri_scan_line_intersection_rgb_triple(int triangle_egde_number,
	int scan_line_y,int *vertex_x,int *vertex_y,float *gradient,
	float *inv_edge_y_length,unsigned char *vertex_rgb_value,int *x_intersection,
	unsigned char *rgb_value)
/*******************************************************************************
LAST MODIFIED 22 February 2002

DESCRIPTION :
Given <triangle_egde_number> (from 0-2) and scan_line_y, the y value of the
intersecting "scan line" and <vertex_x> <vertex_y>  arrays of length 3 and
<vertex_rgb_value> array of length 9 (rgb for 3 points) of the x,y, rgb value at
the triangle's vertices and
<gradient> an array of length 3 of the gradients of the triangle's edges,
and <inv_edge_y_length> an array of length 3 of the 1/(triangle's edges y length)
returns <x_intersection>, the x value of the intersection
of the triangle edge with the "scan line" and <rgb_value>, the 3 interpolated rgb
values at this point (x_intersection,y).
Returns an error if scan_line_y isn't btwn the y's of the triangle edge's ends,
or if the y's of the triangle edge's end are the same, i.e you're supposed to
check for these conditions before calling this function.
This function is a quite specific (i.e. non-generic) helper function, called by
construct_rgb_triple_gouraud_map
cf get_triangle_scan_line_intersection
*******************************************************************************/
{
	float grad,inverse_y_length,value_0r,value_0g,value_0b,value_1r,value_1g,
		value_1b;
	int a_x,b_x,a_y,b_y,end_vertex_num,return_code,start_vertex_num;

	ENTER(get_tri_scan_line_intersection_rgb_triple);
	if ((triangle_egde_number>-1)&&(triangle_egde_number<3)&&vertex_x&&vertex_y&&
		gradient&&inv_edge_y_length&&vertex_rgb_value&&x_intersection&&rgb_value)
	{
		return_code=1;
		start_vertex_num=triangle_egde_number;
		end_vertex_num=
			get_triangle_line_end_vertex_num_from_start_num(start_vertex_num);
		a_x=vertex_x[start_vertex_num];
		a_y=vertex_y[start_vertex_num];
		b_x=vertex_x[end_vertex_num];
		b_y=vertex_y[end_vertex_num];
		if (a_y!=b_y)
		{
			if (((scan_line_y>=a_y)&&(scan_line_y<=b_y))||
				((scan_line_y>=b_y)&&(scan_line_y<=a_y)))
			{
				grad=gradient[triangle_egde_number];
				inverse_y_length=inv_edge_y_length[triangle_egde_number];
				/*line_start_x or	line_end_x	*/
				if (a_x==b_x)
				{
					/*infinite gradient*/
					*x_intersection=a_x;
				}
				else
				{
					*x_intersection=(((float)scan_line_y-(float)a_y)/grad)+a_x;
				}
				/*rgb values at start of triangle edge*/
				value_0r=vertex_rgb_value[start_vertex_num*3];
				value_0g=vertex_rgb_value[(start_vertex_num*3)+1];
				value_0b=vertex_rgb_value[(start_vertex_num*3)+2];
				/*value at end of triangle edge*/
				value_1r=vertex_rgb_value[end_vertex_num*3];
				value_1g=vertex_rgb_value[(end_vertex_num*3)+1];
				value_1b=vertex_rgb_value[(end_vertex_num*3)+2];
				/* inverse_y_length= 1/(b_y-ay), but we cached it earlier	*/
				rgb_value[0]=value_1r*(((float)scan_line_y-(float)a_y)*inverse_y_length)+
					value_0r*(((float)b_y-(float)scan_line_y)*inverse_y_length);
				rgb_value[1]=value_1g*(((float)scan_line_y-(float)a_y)*inverse_y_length)+
					value_0g*(((float)b_y-(float)scan_line_y)*inverse_y_length);
				rgb_value[2]=value_1b*(((float)scan_line_y-(float)a_y)*inverse_y_length)+
					value_0b*(((float)b_y-(float)scan_line_y)*inverse_y_length);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_tri_scan_line_intersection_rgb_triple. scan_line_y not btwn a_y and b_y ");
				return_code=0;
			}
		}/* if (a_y!=b_y) */
		else
		{
			display_message(ERROR_MESSAGE,
			"get_tri_scan_line_intersection_rgb_triple. ay==by ");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_tri_scan_line_intersection_rgb_triple. Invalid arguments");
		return_code=0;
	}
	if (return_code==0)
	{
		rgb_value[0]=0;
		rgb_value[1]=0;
		rgb_value[2]=0;
		*x_intersection=0;
	}
	LEAVE;
	
	return (return_code);
}/* get_tri_scan_line_intersection_rgb_triple */

static int construct_rgb_triple_gouraud_map(int image_width,int number_of_triangles,
	int *triangles,int *electrode_x,int*electrode_y,unsigned char *electrode_rgbs,
	unsigned char *texture)
/*******************************************************************************
LAST MODIFIED 26 February 2002

DESCRIPTION :
Construct a <texture> of rgbs  using gouraud shading btwn the electrodes
specified by <electrode_x> <electrode_y>, <electrode_rgbs>, connected by
<triangles>, <number_of_triangles>. <triangles> contains indices into
<electrode_x> <electrode_y>, <electrode_rgbs>.
cf draw_2d_construct_2d_gouraud_image_map, which works with floats not
rgb triples.
*******************************************************************************/
{
	unsigned char vertex_rgb_value[9]/*rgb for 3 vertices*/,start_rgb_value[3],
		end_rgb_value[3];
	float delta_rgb_value[3],delta_y,gradient[3],
		inv_edge_y_length[3],rgb_value[3];
	int adjusted_x,end_vertex_num,found,i,index,
		intersected_edges[3],k,line_start_x,line_end_x,line_length,
		number_of_intersected_edges,return_code,start_vertex_num,
		triangle_egde_number,tri_min_x,tri_min_y,tri_max_x,
		tri_max_y,tri_width,vertex_x[3],vertex_y[3],x,y;
#if  defined (DEBUG)
	int edge_flag;
#endif 	/*  defined (DEBUG)	*/

	ENTER(construct_rgb_triple_gouraud_map);
	if (electrode_x&&electrode_y&&electrode_rgbs&&texture&&triangles)
	{
		k=0;
		while(k<number_of_triangles*3)
		{
			/* for each triangle vertex*/
			for(i=0;i<3;i++)
			{
				index=triangles[k];
				vertex_x[i]=electrode_x[index];
				vertex_y[i]=electrode_y[index];
				/*rgb*/
				vertex_rgb_value[i*3]=electrode_rgbs[index*3];
				vertex_rgb_value[(i*3)+1]=electrode_rgbs[(index*3)+1];
				vertex_rgb_value[(i*3)+2]=electrode_rgbs[(index*3)+2];
				/*get min and max x and y of triangle*/
				if (i==0)
				{
					tri_min_x=tri_max_x=vertex_x[0];
					tri_min_y=tri_max_y=vertex_y[0];
				}
				if (vertex_x[i]>tri_max_x)
				{
					tri_max_x=vertex_x[i];
				}
				if (vertex_x[i]<tri_min_x)
				{
					tri_min_x=vertex_x[i];
				}
				if (vertex_y[i]>tri_max_y)
				{
					tri_max_y=vertex_y[i];
				}
				if (vertex_y[i]<tri_min_y)
				{
					tri_min_y=vertex_y[i];
				}
				k++;
			}/* for(i=0;i<3;i++)*/
			/*wide  triangles have wrapped around the back and in cp coords, crossed*/
			/*PI/-PI. Extend these wrapped triangles in rc x, so that they'll be drawn */
			/*at the edge of the map, not behind it.*/
			tri_width=abs(tri_max_x-tri_min_x);
			if (tri_width>(image_width/2))
			{
				for(i=0;i<3;i++)
				{
					if (vertex_x[i]<(image_width/2))
					{
						vertex_x[i]+=image_width;
					}
				}
			}
			/*determine and cache the gradient and inverse of the y_length of the*/
			/* triangle's edges*/
			for(i=0;i<3;i++)
			{
				start_vertex_num=i;
				end_vertex_num=
					get_triangle_line_end_vertex_num_from_start_num(start_vertex_num);
				delta_y=(float)vertex_y[end_vertex_num]-(float)vertex_y[start_vertex_num];
				if (vertex_x[start_vertex_num]==vertex_x[end_vertex_num])
				{
					/*infinite gradient. Put in a large number! Anyway, we check for*/
					/* start_x=end_x when consume gradient[] in */
					/*get_tri_scan_line_intersection_rgb_triple, so never use */
					gradient[i]=1000000.0;
				}
				else
				{
					gradient[i]=delta_y/
						((float)vertex_x[end_vertex_num]-(float)vertex_x[start_vertex_num]);
				}
				if (delta_y==0.0)
				{
					/* zero length. Put in a large number for 1/length. This is never used*/
					inv_edge_y_length[i]=1000000.0;
				}
				else
				{
					inv_edge_y_length[i]=1.0/(delta_y);
				}
			}/* for(i=0;i<3;i++) */
			/*for each constant y "scan line"*/
			for(y=tri_min_y;y<=tri_max_y;y++)
			{
				number_of_intersected_edges=0;
				/*check and store which of the triangle's edges  are intersected*/
				/*by the constant y "scan line". There should be 2 or 3 edges */
				/*intersected. 2 edges, including the case of a single vertex,or 3 */
				/*if one of the edges is the "scan line", gradient==0 or if a vertex and */
				/*an edge are intersected. */
				if ((y>=vertex_y[0]&&y<=vertex_y[1])||(y>=vertex_y[1]&&y<=vertex_y[0]))
				{
					/*intersects 1st triangle edge, from vertex 0 to 1*/
					intersected_edges[number_of_intersected_edges]=0;
					number_of_intersected_edges++;
				}
				if ((y>=vertex_y[1]&&y<=vertex_y[2])||(y>=vertex_y[2]&&y<=vertex_y[1]))
				{
					/*intersects 2nd triangle edge from vertex 1 to 2*/
					intersected_edges[number_of_intersected_edges]=1;
					number_of_intersected_edges++;
				}
				if ((y>=vertex_y[2]&&y<=vertex_y[0])||(y>=vertex_y[0]&&y<=vertex_y[2]))
				{
					/*intersects 3rd triangle edgefrom  vertex 2 to 0 */
					intersected_edges[number_of_intersected_edges]=2;
					number_of_intersected_edges++;
				}
				/*find line across triangle*/
				if (number_of_intersected_edges==3)
				{
					/*If one of triangle edges has gradient==0,This is our line across triangle*/
					i=0;
					found=0;
					while((i<3)&&(!found))
					{
						if (gradient[i]==0)
						{
							found=1;
						}
						else
						{
							i++;
						}
					}
					if (found)
					{
						return_code=1;
						start_vertex_num=i;
						end_vertex_num=
							get_triangle_line_end_vertex_num_from_start_num(start_vertex_num);
						line_start_x=vertex_x[start_vertex_num];
						line_end_x=vertex_x[end_vertex_num];
						/*rgb*/
						start_rgb_value[0]=vertex_rgb_value[start_vertex_num*3];
						start_rgb_value[1]=vertex_rgb_value[(start_vertex_num*3)+1];
						start_rgb_value[2]=vertex_rgb_value[(start_vertex_num*3)+2];
						/*rgb*/
						end_rgb_value[0]=vertex_rgb_value[end_vertex_num*3];
						end_rgb_value[1]=vertex_rgb_value[(end_vertex_num*3)+1];
						end_rgb_value[2]=vertex_rgb_value[(end_vertex_num*3)+2];
					}/* if (found) */
					else
					{
						/*2 of the intersections will at a common vertex, the third in the middle*/
						/* of an edge (including the degenerate case, where this middle edge is a*/
						/* point, and the triangle is a line*) */
						for(i=0;i<3;i++)
						{
							triangle_egde_number=intersected_edges[i];
							start_vertex_num=triangle_egde_number;
							end_vertex_num=
								get_triangle_line_end_vertex_num_from_start_num(start_vertex_num);
							if (vertex_y[start_vertex_num]==y)
							{
								return_code=1;
								/*start intersection at a vertex */
								line_start_x=vertex_x[start_vertex_num];
								/*rgb*/
								start_rgb_value[0]=vertex_rgb_value[start_vertex_num*3];
								start_rgb_value[1]=vertex_rgb_value[(start_vertex_num*3)+1];
								start_rgb_value[2]=vertex_rgb_value[(start_vertex_num*3)+2];
								if (vertex_y[end_vertex_num]==y)
								{
									/*end intersection at a vertex */
									line_end_x=vertex_x[end_vertex_num];
									/*rgb*/
									end_rgb_value[0]=vertex_rgb_value[end_vertex_num*3];
									end_rgb_value[1]=vertex_rgb_value[(end_vertex_num*3)+1];
									end_rgb_value[2]=vertex_rgb_value[(end_vertex_num*3)+2];
								}
							}
							else if (vertex_y[end_vertex_num]!=y)
							{
								/*end intersection in the middle of an edge */
								return_code=get_tri_scan_line_intersection_rgb_triple(intersected_edges[i],
									y,vertex_x,vertex_y,gradient,inv_edge_y_length,vertex_rgb_value,
									&line_end_x,end_rgb_value);
							}
						}/* for(i=0;i<3;i++)*/
					} /* if (found) */
				}
				else if (number_of_intersected_edges==2)
				{
					/*both of the intersections in the middle of an edge */
					return_code=(get_tri_scan_line_intersection_rgb_triple(intersected_edges[0],
						y,vertex_x,vertex_y,gradient,inv_edge_y_length,vertex_rgb_value,
						&line_start_x,start_rgb_value)&&
						get_tri_scan_line_intersection_rgb_triple(intersected_edges[1],
							y,vertex_x,vertex_y,gradient,inv_edge_y_length,vertex_rgb_value,
							&line_end_x,end_rgb_value));
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"construct_rgb_triple_gouraud_map. Wrong number of intersections");
					return_code=0;
				}
				if (return_code)
				{
					if (line_end_x==line_start_x)
					{
						/*rgb*/
						delta_rgb_value[0]=0;
						delta_rgb_value[1]=0;
						delta_rgb_value[2]=0;
					}
					else
					{
						line_length=line_end_x-line_start_x;
						/*rgb*/
						delta_rgb_value[0]=(end_rgb_value[0]-start_rgb_value[0])/(line_length);
						delta_rgb_value[1]=(end_rgb_value[1]-start_rgb_value[1])/(line_length);
						delta_rgb_value[2]=(end_rgb_value[2]-start_rgb_value[2])/(line_length);
					}
					/*ensure line_start_x>line_end_x*/
					if (line_start_x>line_end_x)
					{
						x=line_start_x;
						line_start_x=line_end_x;
						line_end_x=x;
						/*rgb*/
						rgb_value[0]=start_rgb_value[0];
						rgb_value[1]=start_rgb_value[1];
						rgb_value[2]=start_rgb_value[2];
						start_rgb_value[0]=end_rgb_value[0];
						start_rgb_value[1]=end_rgb_value[1];
						start_rgb_value[2]=end_rgb_value[2];
						end_rgb_value[0]=rgb_value[0];
						end_rgb_value[1]=rgb_value[1];
						end_rgb_value[2]=rgb_value[2];
					}
					/*do line of pixels across the triangle*/
					/*rgb*/
					rgb_value[0]=start_rgb_value[0];
					rgb_value[1]=start_rgb_value[1];
					rgb_value[2]=start_rgb_value[2];
					for(x=line_start_x;x<=line_end_x;x++)
					{
#if defined (DEBUG)
						/*flag pixels at the triangle's edges*/
						if ((x==line_start_x)||(x==line_end_x)||
							(y==tri_min_y)||(y==tri_max_y))
						{
							edge_flag=1;
						}
						else
						{
							edge_flag=0;
						}
#endif /* defined (DEBUG)*/
						/*ensure the pixels of (wrapped) triangles at RH edge wrap to LH edge*/
						if (x>=(image_width))
						{
							adjusted_x=x-image_width;
						}
						else
						{
							adjusted_x=x;
						}
#if defined (DEBUG)
						/* draw pixels at triangle's edges red, so can see easily*/
						if (edge_flag)
						{
							/*rgb*/
							texture[(y*image_width+adjusted_x)*3]=255;/*red*/
							texture[(y*image_width+adjusted_x)*3+1]=0;
							texture[(y*image_width+adjusted_x)*3+2]=0;
						}
						else
#endif /* defined (DEBUG)*/
						{
							/*rgb*/
							texture[(y*image_width+adjusted_x)*3]=rgb_value[0];
							texture[(y*image_width+adjusted_x)*3+1]=rgb_value[1];
							texture[(y*image_width+adjusted_x)*3+2]=rgb_value[2];
						}
						rgb_value[0]+=delta_rgb_value[0];
						rgb_value[1]+=delta_rgb_value[1];
						rgb_value[2]+=delta_rgb_value[2];
					}	/* for(x=line_start_x;x<=line_end_x;x++) */
				}/* if (return_code)*/
			}/* for(y=tri_min_y;y<=tri_max_y;y++) */
		}/* (k=0;k<number_of_triangles;k++) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"construct_rgb_triple_gouraud_map. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* construct_rgb_triple_gouraud_map*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int make_skin_texture(unsigned char *texture_image,
	int texture_x_length,int texture_y_length,unsigned char skin_r,
	unsigned char skin_g,unsigned char skin_b)
/*******************************************************************************
LAST MODIFIED : 7 March 2002

DESCRIPTION : Given the (already allocated) <texture_image> of rgb triples size
<image_x_length>*<image_y_length>*3,
fills in the texture with the colour  specified by <skin_r>, <skin_g>, <skin_b>
==============================================================================*/
{
	unsigned char *texture_image_ptr;
	int i,j,return_code;

	ENTER(make_skin_and_electrodes_texture);
	if (texture_image)
	{
		return_code=1;
		/*pink skin colour*/
		texture_image_ptr=texture_image;
		for(i=0;i<texture_y_length;i++)
		{
			for(j=0;j<texture_x_length;j++)
			{
				*texture_image_ptr=skin_r;
				texture_image_ptr++;
				*texture_image_ptr=skin_g;
				texture_image_ptr++;
				*texture_image_ptr=skin_b;
				texture_image_ptr++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_skin_and_electrodes_texture. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}	/* make_skin_and_electrodes_texture */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int make_skin_and_electrodes_texture(unsigned char *texture_image,
	int *electrode_x,int *electrode_y,unsigned char *electrode_rgbs,
	int image_x_length,	int image_y_length,int number_of_electrodes,
	unsigned char skin_r,unsigned char skin_g,unsigned char skin_b)
/*******************************************************************************
LAST MODIFIED : 26 February 2001

DESCRIPTION : Given the (already allocated) <texture_image> of rgb triples size
<image_x_length>*<image_y_length>*3, and <number_of_electrodes> electrodes
specified by <electrode_x> <electrode_y> (range 0->image_>_length),
fills in the texture with the colour  specified by <skin_r>, <skin_g>, <skin_b>
and draws the electrodes with the colours in <electrode_rgbs>
(size 3*number_of_electrodes).
==============================================================================*/
{
	int k,return_code;
	unsigned char *texture_image_ptr;

	ENTER(make_skin_and_electrodes_texture);
	if (texture_image&&electrode_x&&electrode_y)
	{
		return_code=make_skin_texture(texture_image,image_x_length,
			image_y_length,skin_r,skin_g,skin_b);
		/*electrodes colours */
		for(k=0;k<number_of_electrodes;k++)
		{
			texture_image_ptr=texture_image+
				(electrode_x[k]+electrode_y[k]*image_x_length)*3;
			*texture_image_ptr=electrode_rgbs[k*3];
			texture_image_ptr++;
			*texture_image_ptr=electrode_rgbs[(k*3)+1];
			texture_image_ptr++;
			*texture_image_ptr=electrode_rgbs[(k*3)+2];
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_skin_and_electrodes_texture. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* make_skin_and_electrodes_texture */
#endif/* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static struct Texture *make_gouraud_texture_from_torso_map(struct Map *map,
	struct Region *region,FE_value time,FE_value z_min,FE_value z_range,
	float minimum,float maximum,int texture_x_length,int texture_y_length,
	int draw_skin)
/*******************************************************************************
LAST MODIFIED : 8 March 2002

DESCRIPTION :
Given <map>,<region>,<time>, <z_min> and <z_range>, <minimum> and <maximum>
the signal min max at <time> and creates <the_skin_texture> of size
<texture_x_length>, <texture_x_length>. This texture is a gouraud shading
of the rgb values  at the map's electrodes.
Will work only with torso maps.

Note: texture_x_length,texture_y_length should bed 2^n, eg 128,256,512 for
opengl texture reasons (at least on the SGI).

==============================================================================*/
{
	enum Texture_storage_type storage_type;
	FE_value value,red,green,blue,x,y,z_rc,r,theta,z_cp,x_offset,y_offset;
	float *vertices;
	int bytes_per_pixel, i, number_of_bytes_per_component, number_of_components,
		number_of_electrodes, nodes_rejected_or_accepted, skin_texture_is_managed,
		success;
	struct Colour black = {0,0,0},result;
	struct Computed_field *scaled_offset_signal_value_at_time_field;
	struct FE_field *electrode_postion_field;
	struct FE_field_component component;
	struct FE_node *electrode_node;
	struct FE_node_order_info *node_order_info;
	struct Graphical_material *material;
	struct GROUP(FE_node) *unrejected_node_group;
	struct Map_3d_package *map_3d_package;
	struct Map_drawing_information *drawing_information;
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *spectrum,*spectrum_to_be_modified_copy;
	struct Texture *skin_texture;
	struct Unemap_package *unemap_package;
	struct Vertices_data vertices_data;
	unsigned char *the_rgb;
	unsigned short blue_short,green_short,red_short;

	ENTER(make_gouraud_texture_from_torso_map);
	vertices=(float *)NULL;
	scaled_offset_signal_value_at_time_field=(struct Computed_field *)NULL;
	electrode_postion_field=(struct FE_field *)NULL;
	electrode_node=(struct FE_node *)NULL;
	material=(struct Graphical_material *)NULL;
	texture_manager=(struct MANAGER(Texture) *)NULL;
	unrejected_node_group=(struct GROUP(FE_node) *)NULL;
	spectrum=(struct Spectrum *)NULL;
	spectrum_to_be_modified_copy=(struct Spectrum *)NULL;
	spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
	skin_texture=(struct Texture *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	node_order_info=(struct FE_node_order_info *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	if (map&&region&&(region->type==TORSO)&&(unemap_package=map->unemap_package)&&
		(unrejected_node_group=get_Region_unrejected_node_group(region))&&
		(drawing_information=map->drawing_information)&&
		(texture_manager=drawing_information->texture_manager)&&
		(spectrum=drawing_information->spectrum)&&
		(spectrum_manager=get_map_drawing_information_spectrum_manager(drawing_information)))
	{
		success=1;
		nodes_rejected_or_accepted=
			get_map_drawing_information_electrodes_accepted_or_rejected(drawing_information);
		map_3d_package=get_Region_map_3d_package(region);
		if (nodes_rejected_or_accepted)
		{
			/* free up existing information */
			if (map->triangle_electrode_indices)
			{
				DEALLOCATE(map->triangle_electrode_indices);
			}
			if (map->electrode_tex_x)
			{
				DEALLOCATE(map->electrode_tex_x);
			}
			if (map->electrode_tex_y)
			{
				DEALLOCATE(map->electrode_tex_y);
			}
			if (map->electrode_rgbs)
			{
				DEALLOCATE(map->electrode_rgbs);
			}
			if (map->texture_image)
			{
				DEALLOCATE(map->texture_image);
			}
			/* need to delaunay/gouraud information */
			x_offset=(texture_x_length)/2;/*-(-PI*texture_x_length/2PI)*/
			y_offset=-((z_min*(texture_y_length))/z_range);
			electrode_postion_field=get_Region_electrode_position_field(region);
			component.field=electrode_postion_field;
			number_of_electrodes=(NUMBER_IN_GROUP(FE_node)(unrejected_node_group));
			if (ALLOCATE(vertices,float,/*x,y,z*/3*number_of_electrodes)&&
				ALLOCATE(map->electrode_tex_x,int,number_of_electrodes)&&
				ALLOCATE(map->electrode_tex_y,int,number_of_electrodes)&&
				ALLOCATE(map->electrode_rgbs,unsigned char,/*r,g,b*/3*number_of_electrodes)&&
				ALLOCATE(map->texture_image,unsigned char,
					3*texture_x_length*texture_y_length))
			{
				map->texture_x_length=texture_x_length;
				map->texture_y_length=texture_y_length;
				/* put the nodal position field data into the vertex array*/
				vertices_data.current_element=0;
				vertices_data.vertices=vertices;
				vertices_data.position_field=electrode_postion_field;
				if ((success=FOR_EACH_OBJECT_IN_GROUP(FE_node)
					(put_electrode_pos_into_vertices,
						(void *)&vertices_data,unrejected_node_group))&&
					/*perform the delaunay triangularisation*/
					(success=cylinder_delaunay(number_of_electrodes,vertices,
						&map->number_of_gouraud_triangles,&map->triangle_electrode_indices)))
				{
					/* make a node_order_info, so can match nodes to vertices  */
					if ((node_order_info=CREATE(FE_node_order_info)(0))&&
						(FOR_EACH_OBJECT_IN_GROUP(FE_node)(fill_FE_node_order_info,
							(void *)node_order_info,unrejected_node_group)))
					{
						set_map_3d_package_node_order_info(map_3d_package,node_order_info);
						/* set up array of  map->electrode_tex_x,y */
						for(i=0;i<number_of_electrodes;i++)
						{
							electrode_node=get_FE_node_order_info_node(node_order_info,i);
							/*set up map->electrode_tex_x, map->electrode_tex_y*/
							component.number=0;
							get_FE_nodal_FE_value_value(electrode_node,&component,0,FE_NODAL_VALUE,
								/*time*/0,&x);
							component.number=1;
							get_FE_nodal_FE_value_value(electrode_node,&component,0,FE_NODAL_VALUE,
								/*time*/0,&y);
							component.number=2;
							get_FE_nodal_FE_value_value(electrode_node,&component,0,FE_NODAL_VALUE,
								/*time*/0,&z_rc);
							cartesian_to_cylindrical_polar(x,y,z_rc,&r,&theta,&z_cp,(float *)NULL);
							map->electrode_tex_x[i]=
								(int)(((texture_x_length-1)/(2*PI))*theta+x_offset);
							map->electrode_tex_y[i]=
								(int)(((texture_y_length-1)/z_range)*z_cp+y_offset);
						}	/* for(i=0;i<number_of_electrodes;i++) */
					}
					else
					{
						success=0;
						display_message(ERROR_MESSAGE,"make_gouraud_texture_from_torso_map. "
							"CREATE(FE_node_order_info/"
							"fill_FE_node_order_info failed");
					}
				}	/* if ((success=FOR_EACH_OBJECT_IN_GROUP(FE_node)	 */
			}	/*	if (ALLOCATE(vertices,float, */
			else
			{
				success=0;
				display_message(ERROR_MESSAGE,"make_gouraud_texture_from_torso_map."
					" out of memory for vertices etc");
			}
		} /* if (nodes_rejected_or_accepted) */
		/* alter the spectrum for spectrum_render_value_on_material below */
		if (success)
		{
			if (IS_MANAGED(Spectrum)(spectrum,spectrum_manager))
			{
				if (spectrum_to_be_modified_copy=CREATE(Spectrum)
					("spectrum_modify_temp"))
				{
					MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
						(spectrum_to_be_modified_copy,spectrum);
					Spectrum_set_minimum_and_maximum(spectrum_to_be_modified_copy,
						minimum,maximum);
					MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
						spectrum_to_be_modified_copy,spectrum_manager);
					DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"make_gouraud_texture_from_torso_map.  Could not create spectrum copy.");
					success=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"make_gouraud_texture_from_torso_map.  Spectrum is not in manager!");
				success=0;
			}
		}/* if (success)*/
		if (success)
		{
			/*create a material to get the rgb values on */
			if (material = CREATE(Graphical_material)("value_to_rgb_material"))
			{
				success=Graphical_material_set_diffuse(material,&black);
			}
			else
			{
				success=0;
			}
		}
		if (success)
		{
			node_order_info=get_map_3d_package_node_order_info(map_3d_package);
			number_of_electrodes=get_FE_node_order_info_number_of_nodes(node_order_info);
			scaled_offset_signal_value_at_time_field=
				get_unemap_package_scaled_offset_signal_value_at_time_field(unemap_package);
			/* set up array of rgbs at electrodes */
			the_rgb=map->electrode_rgbs;
			for(i=0;i<number_of_electrodes;i++)
			{
				/*set up rgb values */
				electrode_node=get_FE_node_order_info_node(node_order_info,i);
				Computed_field_evaluate_at_node(scaled_offset_signal_value_at_time_field,
					electrode_node,time,&value/*assume 1 component*/);
				/* convert pixel_value float data to RGB*/
				spectrum_render_value_on_material(spectrum,material,
					/*number_of_data_components*/1,&value);
				Graphical_material_get_diffuse(material,&result);
				red = result.red;
				green = result.green;
				blue = result.blue;
				red_short=(unsigned short)(255*red);
				*the_rgb=red_short;
				the_rgb++;
				green_short=(unsigned short)(255*green);
				*the_rgb=green_short;
				the_rgb++;
				blue_short=(unsigned short)(255*blue);
				*the_rgb=blue_short;
				the_rgb++;
			}	/* for(i=0;i<number_of_electrodes;i++) */
			Computed_field_clear_cache(scaled_offset_signal_value_at_time_field);
			if (draw_skin)
			{
				/* draw a pink texture before adding the potential map.*/
				/* do this as their may be gaps in the texture generated by */
				/*construct_rgb_triple_gouraud_map. This will low things slightly*/
				/* and is unnecessary if there are no gaps in the potential map texture*/
				success=make_skin_texture(map->texture_image,
					texture_x_length,texture_y_length,255,128,128);
			}
			success=construct_rgb_triple_gouraud_map(texture_x_length,
				map->number_of_gouraud_triangles,
				map->triangle_electrode_indices,map->electrode_tex_x,map->electrode_tex_y,
				map->electrode_rgbs,map->texture_image);
		}
		if (success)
		{
			/* find CMGUI texture, and set it with the map->texture_image */
			if (skin_texture = FIND_BY_IDENTIFIER_IN_MANAGER(Texture, name)(
				"2d_unemap", texture_manager))
			{
				/* texture will retain its other properties, eg combine_mode */
				skin_texture_is_managed = 1;
			}
			else
			{
				skin_texture = CREATE(Texture)("2d_unemap");
				Texture_set_filter_mode(skin_texture, TEXTURE_NEAREST_FILTER);
				Texture_set_combine_mode(skin_texture, TEXTURE_MODULATE);
				skin_texture_is_managed = 0;
			}
			if (skin_texture)
			{
				if (skin_texture_is_managed)
				{
					MANAGER_BEGIN_CHANGE(Texture)(texture_manager,
						MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Texture), skin_texture);
				}
				storage_type = TEXTURE_RGB;
				number_of_components =
					Texture_storage_type_get_number_of_components(storage_type);
				number_of_bytes_per_component = 1;
				bytes_per_pixel = number_of_components*number_of_bytes_per_component;
				success =
					Texture_allocate_image(skin_texture,
						/*width*/texture_x_length, /*height*/texture_y_length, /*depth*/1,
						storage_type, number_of_bytes_per_component, (char *)NULL) &&
					Texture_set_image_block(skin_texture, /*left*/0, /*bottom*/0,
						/*width*/texture_x_length, /*height*/texture_y_length,
						/*depth_plane*/0,
						/*source_width_bytes*/texture_x_length*bytes_per_pixel,
						map->texture_image);
				if (skin_texture_is_managed)
				{
					MANAGER_END_CHANGE(Texture)(texture_manager);
				}
				else
				{
					if (!(success &&
						ADD_OBJECT_TO_MANAGER(Texture)(skin_texture, texture_manager)))
					{
						DESTROY(Texture)(&skin_texture);
						skin_texture = (struct Texture *)NULL;
					}
				}
			}
			if (!skin_texture)
			{
				display_message(ERROR_MESSAGE,
					"make_gouraud_texture_from_torso_map.  Could not create texture");
				success = 0;
			}
		}/* if (success) */
		if (material)
		{
			DESTROY(Graphical_material)(&material);
		}
		if (vertices)
		{
			DEALLOCATE(vertices);
		}
		if (!success)
		{
			/* free any set up info, as we failed */
			if (map->triangle_electrode_indices)
			{
				DEALLOCATE(map->triangle_electrode_indices);
			}
			if (map->electrode_tex_x)
			{
				DEALLOCATE(map->electrode_tex_x);
			}
			if (map->electrode_tex_y)
			{
				DEALLOCATE(map->electrode_tex_y);
			}
			if (map->electrode_rgbs)
			{
				DEALLOCATE(map->electrode_rgbs);
			}
			if (map->texture_image)
			{
				DEALLOCATE(map->texture_image);
			}
			map->texture_x_length=0;
			map->texture_y_length=0;
			map->number_of_gouraud_triangles=0;
		}/* if (!success) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_gouraud_texture_from_torso_map . Invalid arguments");
		success=0;
	}
	LEAVE;
	
	return (skin_texture);
}/* make_gouraud_texture_from_torso_map */
#endif/* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
struct Computed_field *make_texture_coord_computed_field(
	struct Unemap_package *unemap_package,char *cp_coords_name,
	FE_value z_min,FE_value z_range)
/*******************************************************************************
LAST MODIFIED : 27 February 2002

DESCRIPTION :
Make and return the Computed field texture_coordinates given <cp_coords_name>,
the name of the cylindrical polar computed field, <z_min> and <z_range>
==============================================================================*/
{
	int success;
	struct Computed_field *cylindrical,*scale_factors,
		*normalised_cylindrical,*reordered,*texture_coords;
	struct MANAGER(Computed_field) *computed_field_manager;
	FE_value scalers[3],offsets[3];
	struct Coordinate_system coordinate_system;

	ENTER(make_texture_coord_computed_field);
	cylindrical=(struct Computed_field *)NULL;
	scale_factors=(struct Computed_field *)NULL;
	normalised_cylindrical=(struct Computed_field *)NULL;
	reordered=(struct Computed_field *)NULL;
	texture_coords=(struct Computed_field *)NULL;
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	if (unemap_package&&(computed_field_manager=computed_field_manager=
		get_unemap_package_Computed_field_manager(unemap_package)))
	{
		success=1;
		/*set up the computed fields to put the texture on the mapped_torso*/
		if (cylindrical=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)
			(cp_coords_name,computed_field_manager))
		{
			/*assume that if computed_field "um_texture_coords" was undefined,*/
			/*so are the rest*/
			if (!(texture_coords=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)
				("um_texture_coords",computed_field_manager)))
			{
				coordinate_system.type=CYLINDRICAL_POLAR;
				if (scale_factors=CREATE(Computed_field)("um_scale_factors"))
				{
					coordinate_system.type=RECTANGULAR_CARTESIAN;
					scalers[0]=1;
					scalers[1]= 2*PI;
					scalers[2]=z_range+1;
					if (!(Computed_field_set_read_only(scale_factors)&&
						Computed_field_set_coordinate_system(scale_factors,
							&coordinate_system)&&
						Computed_field_set_type_constant(scale_factors,
							3,scalers)&&
						ADD_OBJECT_TO_MANAGER(Computed_field)(scale_factors,
							computed_field_manager)))
					{
						DESTROY(Computed_field)(&scale_factors);
						success=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"make_texture_coord_computed_field. failed to create um_scale_factors ");
					success=0;
				}
				if (success)
				{
					if (normalised_cylindrical=CREATE(Computed_field)("um_normalised_cylindrical"))
					{
						coordinate_system.type=RECTANGULAR_CARTESIAN;

						if (!(Computed_field_set_read_only(normalised_cylindrical)&&
							Computed_field_set_coordinate_system(normalised_cylindrical,
								&coordinate_system)&&
							Computed_field_set_type_divide_components(normalised_cylindrical,
								cylindrical,scale_factors)&&
							ADD_OBJECT_TO_MANAGER(Computed_field)(normalised_cylindrical,
								computed_field_manager)))
						{
							DESTROY(Computed_field)(&normalised_cylindrical);
							success=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"make_texture_coord_computed_field. failed to create um_normalised_cylindrical");
						success=0;
					}
				}
				if (success)
				{
					if (reordered=CREATE(Computed_field)("um_reordered"))
					{
						int source_field_numbers[3] = { 0, 0, 0 };
						int source_value_numbers[3] = { 1, 2, 0 };

						coordinate_system.type=RECTANGULAR_CARTESIAN;
						if (!(Computed_field_set_read_only(reordered)&&
							Computed_field_set_coordinate_system(reordered,
								&coordinate_system)&&
							Computed_field_set_type_composite(reordered,
								/*number_of_components*/3,
								/*number_of_source_fields*/1,
								/*source_fields*/&normalised_cylindrical,
								/*number_of_source_values*/0,
								/*source_values*/0,
								source_field_numbers,
								source_value_numbers) &&
							ADD_OBJECT_TO_MANAGER(Computed_field)(reordered,
								computed_field_manager)))
						{
							DESTROY(Computed_field)(&reordered);
							success=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"make_texture_coord_computed_field. failed to create um_reordered");
						success=0;
					}
				}
				if (success)
				{
					if (texture_coords=CREATE(Computed_field)("um_texture_coords"))
					{
						offsets[0]=0.5;
						offsets[1]=-(z_min/z_range);
						offsets[2]=0.5;
						coordinate_system.type=RECTANGULAR_CARTESIAN;
						if (!(Computed_field_set_read_only(texture_coords)&&
							Computed_field_set_coordinate_system(texture_coords,
								&coordinate_system)&&
							Computed_field_set_type_offset(texture_coords,reordered,offsets)&&
							ADD_OBJECT_TO_MANAGER(Computed_field)(texture_coords,
								computed_field_manager)))
						{
							DESTROY(Computed_field)(&texture_coords);
							success=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"make_texture_coord_computed_field. failed to create um_texture_coords");
						success=0;
					}
				}/*if (success)*/
			}/* if (texture_coords=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name) */
		}	/* if (!FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)("um_cylindrical",*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_texture_coord_computed_field invalid arguments");
		success=0;
	}
	LEAVE;
	
	return (texture_coords);
}/* make_texture_coord_computed_field */
#endif/* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
static int draw_map_3d(struct Map *map)
/*******************************************************************************
LAST MODIFIED : 8 March 2002

DESCRIPTION :
This function draws the <map> in as a 3D CMGUI scene, for the current region(s).
Removes 3d drawing for non-current region(s).
==============================================================================*/
{
	char *fit_name;
	/* up vector for the scene. */
	double z_up[3]={0.0,0.0,1.0};
	double *up_vector;
	FE_value focus,time,z_min,z_max,z_range;
	struct Device *device;
	struct FE_field *map_position_field;
	struct FE_field_order_info *field_order_info;
	struct Computed_field *data_field,*texture_coords;
	float frame_time,minimum,maximum;
	int default_torso_loaded,delaunay_map,direct_on_smooth_torso,display_all_regions,
		nodes_rejected_or_accepted,range_set,return_code,*times;
	enum Map_type map_type;
	char undecided_accepted;
	struct Map_drawing_information *drawing_information;
	struct Rig *rig;
	struct Region_list_item *region_item;
	struct Region *current_region,*region;
	struct Interpolation_function *function;
	struct Unemap_package *unemap_package;
	struct Scene *scene;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Spectrum *spectrum,*spectrum_to_be_modified_copy;
	struct Texture *skin_texture;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Map_3d_package *map_3d_package;
	struct GROUP(FE_element) *element_group,*mapped_torso_element_group,
		*delaunay_torso_element_group;
	struct GROUP(FE_node) *rig_node_group,*unrejected_node_group;

	ENTER(draw_map_3d);
	buffer=(struct Signal_buffer *)NULL;
	skin_texture=(struct Texture *)NULL;
	fit_name=(char *)NULL;
	field_order_info=(struct FE_field_order_info *)NULL;
	map_position_field=(struct FE_field *)NULL;
	data_field=(struct Computed_field *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	rig=(struct Rig *)NULL;
	region_item=(struct Region_list_item *)NULL;
	current_region=(struct Region *)NULL;
	region=(struct Region *)NULL;
	function=(struct Interpolation_function *)NULL;
	unemap_package=(struct Unemap_package *)NULL;
	signal=(struct Signal *)NULL;
	scene=(struct Scene *)NULL;
	spectrum=(struct Spectrum *)NULL;
	spectrum_to_be_modified_copy=(struct Spectrum *)NULL;
	spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
	times=(int *)NULL;
	map_3d_package=(struct Map_3d_package *)NULL;
	element_group=(struct GROUP(FE_element) *)NULL;
	rig_node_group=(struct GROUP(FE_node) *)NULL;
	unrejected_node_group=(struct GROUP(FE_node) *)NULL;
	mapped_torso_element_group=(struct GROUP(FE_element) *)NULL;
	delaunay_torso_element_group=(struct GROUP(FE_element) *)NULL;
	if (map&&(drawing_information=map->drawing_information))
	{
		/*destroy the (2d) sub_maps */
		/*??JW need to sort out sub maps for 3D*/
		map_destroy_Sub_maps(map);
		range_set=0;
		display_all_regions=0;
		unemap_package=map->unemap_package;
		/* do we have a default torso loaded ? */
		if (get_unemap_package_torso_group_name(unemap_package))
		{
			default_torso_loaded=1;
		}
		else
		{
			default_torso_loaded=0;
		}
		if (map->type)
		{
			map_type= *(map->type);
		}
		else
		{
			map_type=NO_MAP_FIELD;
		}
		/* have any electrode nodes been accepted or rejected recently?*/
		nodes_rejected_or_accepted=
			get_map_drawing_information_electrodes_accepted_or_rejected
			(drawing_information);
		spectrum=drawing_information->spectrum;
		spectrum_manager=get_map_drawing_information_spectrum_manager(drawing_information);
		if ((map->rig_pointer)&&(rig= *(map->rig_pointer)))
		{
			return_code=1;
			device=*(rig->devices);
			signal=(device)->signal;
			buffer=signal->buffer;
			times=buffer->times;
			frame_time=(int)((float)((times)[*(map->potential_time)])*1000./buffer->frequency);
			undecided_accepted=map->undecided_accepted;
			current_region=get_Rig_current_region(rig);
			/*if current_region NULL, displaying all regions*/
			if (!current_region)
			{
				display_all_regions=1;
			}
			if (map_type!=NO_MAP_FIELD)
			{
				scene=get_map_drawing_information_scene(drawing_information);
				region_item=get_Rig_region_list(rig);
				time=frame_time/1000;/* ms to s*/
				while(region_item)
				{
					region=get_Region_list_item_region(region_item);
					rig_node_group=get_Region_rig_node_group(region);
					unrejected_node_group=get_Region_unrejected_node_group(region);
					map_3d_package=get_Region_map_3d_package(region);
					if ((default_torso_loaded)&&(region->type==TORSO)&&
						(map->interpolation_type==DIRECT_INTERPOLATION))
					{
						direct_on_smooth_torso=1;
						delaunay_map =0;
					}
					else
					{
						delaunay_map = map_is_delaunay(map);
						direct_on_smooth_torso=0;
					}

					/* free everything except the current region(s) map_3d_package*/
					if ((region!=current_region)&&(!display_all_regions))
					{
						/*this will do a deaccess*/
						set_Region_map_3d_package(region,(struct Map_3d_package *)NULL);
						drawing_information->viewed_scene=0;
					}
					/*draw the current region(s) */
					else if (((region==current_region)||display_all_regions)&&
						(unemap_package_rig_node_group_has_electrodes(unemap_package,
							unrejected_node_group)))
					{
						/* create the mapping fields */
						if (region->type==SOCK)
						{
							focus=FIT_SOCK_FOCUS;
						}
						else
						{
							focus=0;
						}
						field_order_info=create_mapping_fields(region->type,focus,unemap_package);
						if (map->interpolation_type==NO_INTERPOLATION)
						{
							/* no surface, just electrodes */
							map_3d_package=get_Region_map_3d_package(region);
							if (map_3d_package)
							{
								element_group=get_map_3d_package_element_group(map_3d_package);
								mapped_torso_element_group=
									get_map_3d_package_mapped_torso_element_group(map_3d_package);
								delaunay_torso_element_group=
									get_map_3d_package_delaunay_torso_element_group(map_3d_package);
							}
							/* if no map_3d_package or last one wasn't NO_INTERPOLATION, */
							/* create a map_3d_package and destroy the any old one*/
							if ((!map_3d_package)||(map_3d_package&&(element_group||
								mapped_torso_element_group||delaunay_torso_element_group)))
							{
								map_3d_package=CREATE(Map_3d_package)(
									get_unemap_package_FE_field_manager(unemap_package),
									get_unemap_package_element_group_manager(unemap_package),
									get_unemap_package_data_manager(unemap_package),
									get_unemap_package_data_group_manager(unemap_package),
									get_unemap_package_node_manager(unemap_package),
									get_unemap_package_node_group_manager(unemap_package),
									get_unemap_package_element_manager(unemap_package),
									get_unemap_package_Computed_field_manager(unemap_package));
								fit_name=make_fit_name(region,rig->name);
								set_map_3d_package_fit_name(map_3d_package,fit_name);
								set_map_3d_package_number_of_map_rows(map_3d_package,
									map->finite_element_mesh_rows);
								set_map_3d_package_number_of_map_columns(map_3d_package,
									map->finite_element_mesh_columns);
								map_position_field=get_FE_field_order_info_field(field_order_info,0);
								set_map_3d_package_position_field(map_3d_package,map_position_field);
								/* will deaccess the old map_3d_package*/
								set_Region_map_3d_package(region,map_3d_package);
							}
						} /* if (map->interpolation_type==NO_INTERPOLATION)	*/
						else
						{
							if (direct_on_smooth_torso)
							{
								if (nodes_rejected_or_accepted)
								{
									if (map_3d_package)
									{
										/*this will do a deaccess*/
										set_Region_map_3d_package(region,(struct Map_3d_package *)NULL);
									}
									/* Create the new Map_3d_package*/
									map_3d_package=CREATE(Map_3d_package)(
										get_unemap_package_FE_field_manager(unemap_package),
										get_unemap_package_element_group_manager(unemap_package),
										get_unemap_package_data_manager(unemap_package),
										get_unemap_package_data_group_manager(unemap_package),
										get_unemap_package_node_manager(unemap_package),
										get_unemap_package_node_group_manager(unemap_package),
										get_unemap_package_element_manager(unemap_package),
										get_unemap_package_Computed_field_manager(unemap_package));
									set_Region_map_3d_package(region,map_3d_package);
									/*  set up the mapped torso node and element groups */
									make_mapped_torso_node_and_element_groups(region,unemap_package);
									map_define_scaled_offset_signal_at_time(unemap_package);
								}
								if (!map->fixed_range)
								{
									get_rig_node_group_signal_min_max_at_time(unrejected_node_group,
										get_unemap_package_scaled_offset_signal_value_at_time_field(unemap_package),
										get_unemap_package_signal_status_field(unemap_package),
										time,&minimum,&maximum);
								}
								else
								{
									minimum=map->minimum_value;
									maximum=map->maximum_value;
								}
								z_min=get_map_3d_package_electrodes_min_z(map_3d_package);
								z_max=get_map_3d_package_electrodes_max_z(map_3d_package);
								z_range=z_max-z_min;
								skin_texture=make_gouraud_texture_from_torso_map(map,region,time,
									z_min,z_range,minimum,maximum,256/*texture_x_length*/,
									256/*texture_y_length*/,1/*draw_skin*/);
								/* range set in make_gouraud_texture_from_torso_map, so don't*/
								/* want to do it again below*/
								range_set=0;
								texture_coords=make_texture_coord_computed_field(unemap_package,
									"um_cylindrical_polar_coords",z_min,z_range);
							}/* if (direct_on_smooth_torso)*/
							else
							{
								if (delaunay_map)
								{
									/* DIRECT_INTERPOLATION */
									make_and_set_delaunay(region,unemap_package,time,
										nodes_rejected_or_accepted);
								}
								else
								{
									/* BICUBIC_INTERPOLATION */
									if (function=calculate_interpolation_functio(map_type,
										rig,region,	map->event_number,frame_time,map->datum,
										map->start_search_interval,map->end_search_interval,undecided_accepted,
										map->finite_element_mesh_rows,
										map->finite_element_mesh_columns,map->membrane_smoothing,
										map->plate_bending_smoothing))
									{
										/* Now we have the interpolation_function struct */
										/* make the node and element groups from it.*/
										/* 1st node_group is 'all_devices_node_group */
										make_fit_node_and_element_groups(field_order_info,function,
											unemap_package,rig->name,FIT_SOCK_LAMBDA,
											FIT_TORSO_MAJOR_R,FIT_TORSO_MINOR_R,FIT_PATCH_Z,region,
											nodes_rejected_or_accepted);
										destroy_Interpolation_function(&function);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"draw_map_3d. calculate_interpolation_functio failed  ");
										return_code=0;
									}
								}
							}/*direct_on_smooth_torso*/
							/* get map_3d_package again, it may have changed */
							map_3d_package=get_Region_map_3d_package(region);
							if (delaunay_map)
							{
								/* do gouraud torso surface*/
								element_group=
									get_map_3d_package_delaunay_torso_element_group(map_3d_package);
							}
							else
							{
								/* Show the map element surface */
								if (((region->type==TORSO)&&default_torso_loaded)||direct_on_smooth_torso)
								{
									/* do smooth torso surface*/
									element_group=
										get_map_3d_package_mapped_torso_element_group(map_3d_package);
								}
								else
								{
									/* do cylinder surface */
									element_group=get_map_3d_package_element_group(map_3d_package);
								}
							}
							/* if no interpolation, or no spectrum selected(HIDE_COLOUR) don't use them!*/
							if ((map->interpolation_type==NO_INTERPOLATION)||
								(map->colour_option==HIDE_COLOUR))
							{
								/* No Spectrum or computed field used.*/
								map_show_surface(scene,element_group,
									get_map_drawing_information_map_graphical_material
									(drawing_information),
									get_map_drawing_information_Graphical_material_manager
									(drawing_information),
									(struct Spectrum *)NULL,(struct Computed_field *)NULL,
									get_map_drawing_information_no_interpolation_colour
									(drawing_information),
									get_unemap_package_potential_time_object(unemap_package),
									get_map_drawing_information_user_interface(drawing_information),
									delaunay_map,!(map->fixed_range),
									(struct Texture *)NULL,(struct Computed_field *)NULL);
							}
							else if (direct_on_smooth_torso)
							{
								/* texture on smooth torso*/
								map_show_surface(scene,element_group,
									get_map_drawing_information_map_graphical_material
									(drawing_information),
									get_map_drawing_information_Graphical_material_manager
									(drawing_information),
									spectrum,(struct Computed_field *)NULL,
									get_map_drawing_information_no_interpolation_colour
									(drawing_information),
									get_unemap_package_potential_time_object(unemap_package),
									get_map_drawing_information_user_interface(drawing_information),
									1/*direct_interpolation*/,!(map->fixed_range),skin_texture,texture_coords);
							}
							else
							{
								/* BICUBIC_INTERPOLATION or DIRECT_INTERPOLATION */
								data_field = map_get_data_field_for_3d_map(map, map_3d_package);
								map_show_surface(scene,element_group,
									get_map_drawing_information_map_graphical_material
									(drawing_information),
									get_map_drawing_information_Graphical_material_manager
									(drawing_information),
									spectrum,data_field,(struct Colour*)NULL,
									get_unemap_package_potential_time_object(unemap_package),
									get_map_drawing_information_user_interface(drawing_information),
									delaunay_map, !(map->fixed_range),
									(struct Texture *)NULL,(struct Computed_field *)NULL);
							}
						} /*if (map->interpolation_type==NO_INTERPOLATION)*/
						/* can't do electrodes on default_torso surface yet*/
						/*(possibly)  make the map_electrode_position_field, add to the rig nodes*/
						if ((region->type!=TORSO)||(!(default_torso_loaded&&
							(!delaunay_map)&&direct_on_smooth_torso)))
						{
							make_and_add_map_electrode_position_field(unemap_package,
								rig_node_group,region,delaunay_map);
						}
						/*draw (or remove) the arm labels  */
						if ((region->type==TORSO)&&(!default_torso_loaded)&&
							(!delaunay_map))/*FOR AJP*/
						{
							draw_torso_arm_labels(drawing_information,region);
						}
						else
						{
							map_remove_torso_arm_labels(drawing_information);
						}
						DESTROY(FE_field_order_info)(&field_order_info);
					}/* if (unemap_package_rig_node_group_has_electrodes */
					region_item=get_Region_list_item_next(region_item);
				} /* while */
				/* Alter the spectrum */
				/* spectrum range changed and fixed */
				if (map->fixed_range)
				{
					if (map->range_changed)
					{
						map->range_changed=0;
						minimum=map->minimum_value;
						maximum=map->maximum_value;
						range_set=1;
					}
				}
				/* spectrum range automatic */
				else
				{
					/* BICUBIC map range comes from the fitted surface, but NOT the*/
					/*signals(electrodes). Remove the electrode glyphs, so only get */
					/*range from surface(s). Similarly for DIRECT, as the electrodes*/
					/* include rejected signals, whcih will mess up range. */
					/* Also remove electodes for default torso surface, as can't display these yet */
					/* NO_INTERPOLATION-map range comes from the signals (i.e electrodes) */
					if (map->interpolation_type==NO_INTERPOLATION)
					{
						map_define_scaled_offset_signal_at_time(unemap_package);
						get_rig_node_group_signal_min_max_at_time(
							get_Rig_all_devices_rig_node_group(rig),
							get_unemap_package_scaled_offset_signal_value_at_time_field(unemap_package),
							get_unemap_package_signal_status_field(unemap_package),
							time,&minimum,&maximum);
						map->minimum_value=minimum;
						map->maximum_value=maximum;
						map->contour_minimum=minimum;
						map->contour_maximum=maximum;
						map->range_changed=0;
						range_set=1;
					}
					/* BICUBIC_INTERPOLATION	or DIRECT_INTERPOLATION */
					else
					{
						/*nothing to do*/
						;
					}
				}	/* if (map->fixed_range) */
				if (range_set)
				{
					map->range_changed=0;
					if (IS_MANAGED(Spectrum)(spectrum,spectrum_manager))
					{
						if (spectrum_to_be_modified_copy=CREATE(Spectrum)
							("spectrum_modify_temp"))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
								(spectrum_to_be_modified_copy,spectrum);
							Spectrum_set_minimum_and_maximum(spectrum_to_be_modified_copy,
								minimum,maximum);
							MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
								spectrum_to_be_modified_copy,spectrum_manager);
							DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"draw_map_3d.  Could not create spectrum copy.");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"draw_map_3d.  Spectrum is not in manager!");
						return_code=0;
					}
				}
				/* can't do electrodes on default_torso surface yet*/
				if ((region->type!=TORSO)||(!(default_torso_loaded&&(!delaunay_map))))
				{
					map_draw_map_electrodes(unemap_package,map);
				}
				else
				{
					map_remove_all_electrodes(map);
				}
				if (!direct_on_smooth_torso)
				{
					/*can't yet do for this case */
					map_draw_contours(map,spectrum,unemap_package);
				}
				/* First time the scene's viewed  do "view_all"*/
				if (!get_map_drawing_information_viewed_scene(drawing_information))
				{
					/* set z as the up vector for default torso */
					if ((region->type==TORSO)&&(default_torso_loaded))
					{
						up_vector=z_up;
					}
					else
					{
						up_vector=(double *)NULL;
					}
					/* unemap_package_align_scene does Scene_viewer_view_all */
					if (map_drawing_information_align_scene(drawing_information,up_vector))
					{
						/* perturb the lines(for the contours) */
						Scene_viewer_set_perturb_lines(get_map_drawing_information_scene_viewer
							(drawing_information),1);
						set_map_drawing_information_viewed_scene(drawing_information,1);
					}
				}
			}/* if (map_type!=NO_MAP_FIELD) */
		}/* if (map->rig_pointer) */
		/* we've dealt with changes to accepted/rejected signals,so clear flag */
		set_map_drawing_information_electrodes_accepted_or_rejected(drawing_information,0);
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_map_3d.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* draw_map_3d */
#endif /* #if defined (UNEMAP_USE_3D) */

int get_number_of_sub_maps_x_step_y_step_rows_cols(struct Map *map,
	struct Drawing_2d *drawing,int *number_of_sub_maps,int *x_step, int *y_step,
	int *sub_map_rows, int *sub_map_cols)
/*******************************************************************************
LAST MODIFIED : 10 July 2001

DESCRIPTION: Given <map>,<drawing> returns the number_of_sub_maps (based upon the
number of Electrical_imaging_events) and the x_step and y_step, (offsets in the
drawing) and sub_map_rows, sub_map_cols, the number of sub_map rows and columns.
==============================================================================*/
{
	float k;
	int return_code;

	ENTER(get_number_of_sub_maps_x_step_y_step_rows_cols);
	if (map&&drawing)
	{
		return_code=1;
		if (*map->first_eimaging_event)
		{
			*number_of_sub_maps=count_Electrical_imaging_events(*map->first_eimaging_event);
		}
		else
		{
			*number_of_sub_maps=1;
		}
		k=sqrt(*number_of_sub_maps);
		*sub_map_rows=ceil(k);
		*sub_map_cols=ceil((float)(*number_of_sub_maps)/(*sub_map_rows));
		*x_step=drawing->width/(*sub_map_rows);
		*y_step=drawing->height/(*sub_map_cols);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_number_of_sub_maps_x_step_y_step_rows_cols.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* get_number_of_sub_maps_x_step_y_step_rows_cols */

static int draw_2d_fill_in_image(struct Map *map,struct Sub_map *sub_map,
	Pixel *spectrum_pixels)
/*******************************************************************************
LAST MODIFIED : 17 September 2001

DESCRIPTION :
Fill in the 2D map pixel image.
*******************************************************************************/
{
	float background_pixel_value,boundary_pixel_value,f_approx,max_f,min_f,
		*pixel_value,range_f;
	int cell_number,contour_area,contour_areas_in_x,next_contour_x,map_height,
		map_width,number_of_contour_areas,number_of_spectrum_colours,return_code,
		x_pixel,y_pixel;
	Pixel background_pixel,boundary_pixel;
	short int *contour_x,*contour_y;
	struct Map_drawing_information *drawing_information;
	struct Map_frame *frame;
	XImage *frame_image=(XImage *)NULL;

	ENTER(draw_2d_fill_in_image);
	frame=(struct Map_frame *)NULL;
	pixel_value=(float *)NULL;
	contour_x=(short int *)NULL;
	contour_y=(short int *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	if (map&&sub_map&&(drawing_information=map->drawing_information))
	{
		return_code=1;
		map_height=sub_map->height;
		map_width=sub_map->width;
		background_pixel=drawing_information->background_drawing_colour;
		boundary_pixel=drawing_information->boundary_colour;
		contour_areas_in_x=map->number_of_contour_areas_in_x;
		number_of_contour_areas=map->number_of_contour_areas;
		number_of_spectrum_colours=drawing_information->number_of_spectrum_colours;
		/* fill in the image */
		min_f=map->minimum_value;
		max_f=map->maximum_value;
		/* calculate range of values */
		range_f=max_f-min_f;
		if (range_f<=0)
		{
			range_f=1;
		}
		frame=&(sub_map->frame);

		background_pixel_value=sub_map->minimum_value;
		boundary_pixel_value=sub_map->maximum_value;
		pixel_value=frame->pixel_values;
		frame_image=frame->image;
		for (y_pixel=0;y_pixel<map_height;y_pixel++)
		{
			contour_area=
				(y_pixel/map->contour_y_spacing)*contour_areas_in_x;
			next_contour_x=map->contour_x_spacing-1;
			contour_x=(frame->contour_x)+contour_area;
			contour_y=(frame->contour_y)+contour_area;
			for (x_pixel=0;x_pixel<map_width;x_pixel++)
			{
				f_approx= *pixel_value;
				if (f_approx>=background_pixel_value)
				{
					if (f_approx<=boundary_pixel_value)
					{
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
						XPutPixel(frame_image,x_pixel,y_pixel,
							spectrum_pixels[cell_number]);
						cell_number *= number_of_contour_areas;
						contour_x[cell_number]=x_pixel;
						contour_y[cell_number]=y_pixel;
					}
					else
					{
						XPutPixel(frame_image,x_pixel,y_pixel,
							boundary_pixel);
					}
				}
				else
				{
					XPutPixel(frame_image,x_pixel,y_pixel,
						background_pixel);
				}
				pixel_value++;
				if (x_pixel>=next_contour_x)
				{
					contour_x++;
					contour_y++;
					next_contour_x += map->contour_x_spacing;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_fill_in_image invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_fill_in_image */

static int draw_2d_constant_thickness_contours(struct Map *map,
	struct Sub_map *sub_map,struct Drawing_2d *drawing,float *pixel_value,
	char draw_boundary)
/*******************************************************************************
LAST MODIFIED : 19 September 2001

DESCRIPTION :
draw the constant thickness contours
*******************************************************************************/
{
	Display *display;
	GC graphics_context;
	int boundary_type,end,i,j,k,map_x_offset,map_y_offset,map_width,map_height,
		number_of_contours,return_code,start,valid_i_j,valid_i_jm1,valid_im1_j,
		valid_im1_jm1;
	float a,b,background_pixel_value,boundary_pixel_value,contour_step,contour_maximum,
		contour_minimum,f_i_j,f_i_jm1,f_im1_j,f_im1_jm1,min_f,max_f;
	struct Map_drawing_information *drawing_information;

	ENTER(draw_2d_constant_thickness_contours);
	display=(Display *)NULL;
	if (map&&sub_map&&drawing&&(drawing_information=map->drawing_information)&&
		pixel_value)
	{
		return_code=1;
		min_f=map->minimum_value;
		max_f=map->maximum_value;
		map_x_offset=sub_map->x_offset;
		map_y_offset=sub_map->y_offset;
		map_width=sub_map->width;
		map_height=sub_map->height;
		display=User_interface_get_display(drawing->user_interface);
		number_of_contours=map->number_of_contours;
		contour_minimum=map->contour_minimum;
		contour_maximum=map->contour_maximum;

		busy_cursor_on((Widget)NULL,drawing_information->user_interface);
		contour_step=(contour_maximum-contour_minimum)/
			(float)(number_of_contours-1);
		graphics_context=(drawing_information->graphics_context).contour_colour;
		background_pixel_value=sub_map->minimum_value;
		boundary_pixel_value=sub_map->maximum_value;
		for (j=1+map_y_offset;j<map_height+map_y_offset;j++)
		{
			f_i_jm1= *pixel_value;
			if ((background_pixel_value<=f_i_jm1)&&
				(f_i_jm1<=boundary_pixel_value))
			{
				valid_i_jm1=1;
			}
			else
			{
				valid_i_jm1=0;
			}
			f_i_j=pixel_value[map_width];
			if ((background_pixel_value<=f_i_j)&&
				(f_i_j<=boundary_pixel_value))
			{
				valid_i_j=1;
			}
			else
			{
				valid_i_j=0;
			}
			pixel_value++;
			for (i=1+map_x_offset;i<map_width+map_x_offset;i++)
			{
				valid_im1_jm1=valid_i_jm1;
				f_im1_jm1=f_i_jm1;
				valid_im1_j=valid_i_j;
				f_im1_j=f_i_j;
				f_i_jm1= *pixel_value;
				if ((background_pixel_value<=f_i_jm1)&&
					(f_i_jm1<=boundary_pixel_value))
				{
					valid_i_jm1=1;
				}
				else
				{
					valid_i_jm1=0;
				}
				f_i_j=pixel_value[map_width];
				if ((background_pixel_value<=f_i_j)&&
					(f_i_j<=boundary_pixel_value))
				{
					valid_i_j=1;
				}
				else
				{
					valid_i_j=0;
				}
				pixel_value++;
				boundary_type=((valid_im1_jm1*2+valid_im1_j)*2+
					valid_i_jm1)*2+valid_i_j;
				if (15==boundary_type)
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
							if (fabs(a)<0.00001*(fabs(contour_maximum)+
								fabs(contour_minimum)))
							{
								a=0;
							}
							/* dashed lines for -ve contours */
							if ((a>=0)||((i+j)%5<2))
							{
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
											b=(a-f_im1_jm1)*
												(f_im1_jm1+f_i_j-f_im1_j-f_i_jm1)+
												(f_im1_j-f_im1_jm1)*
												(f_i_jm1-f_im1_jm1);
											if (b<0)
											{
												XPSDrawLineFloat(display,
													drawing->pixel_map,graphics_context,
													(float)i-(a-f_i_jm1)/
													(f_im1_jm1-f_i_jm1),(float)(j-1),
													(float)(i-1),(float)j-(a-f_im1_j)/
													(f_im1_jm1-f_im1_j));
												XPSDrawLineFloat(display,
													drawing->pixel_map,graphics_context,
													(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
													(float)j,
													(float)i,
													(float)j-(a-f_i_j)/(f_i_jm1-f_i_j));
											}
											else
											{
												if (b>0)
												{
													XPSDrawLineFloat(display,
														drawing->pixel_map,
														graphics_context,
														(float)i-(a-f_i_jm1)/
														(f_im1_jm1-f_i_jm1),(float)(j-1),
														(float)i,(float)j-(a-f_i_j)/
														(f_i_jm1-f_i_j));
													XPSDrawLineFloat(display,
														drawing->pixel_map,
														graphics_context,
														(float)i-(a-f_i_j)/
														(f_im1_j-f_i_j),
														(float)j,(float)(i-1),(float)j-
														(a-f_im1_j)/(f_im1_jm1-f_im1_j));
												}
												else
												{
													XPSDrawLineFloat(display,
														drawing->pixel_map,
														graphics_context,
														(float)(i-1),(float)j-(a-f_im1_j)/
														(f_im1_jm1-f_im1_j),(float)i,
														(float)j-(a-f_i_j)/
														(f_i_jm1-f_i_j));
													XPSDrawLineFloat(display,
														drawing->pixel_map,
														graphics_context,
														(float)i-(a-f_i_jm1)/
														(f_im1_jm1-f_i_jm1),(float)(j-1),
														(float)i-(a-f_i_j)/
														(f_im1_j-f_i_j),(float)j);
												}
											}
										}
										else
										{
											XPSDrawLineFloat(display,
												drawing->pixel_map,
												graphics_context,(float)i-(a-f_i_jm1)/
												(f_im1_jm1-f_i_jm1),(float)(j-1),
												(float)(i-1),(float)j-(a-f_im1_j)/
												(f_im1_jm1-f_im1_j));
										}
									}
									else
									{
										if (((f_i_jm1<=a)&&(a<f_i_j))||
											((f_i_jm1>=a)&&(a>f_i_j)))
										{
											XPSDrawLineFloat(display,
												drawing->pixel_map,graphics_context,
												(float)i-(a-f_i_jm1)/
												(f_im1_jm1-f_i_jm1),(float)(j-1),
												(float)i,
												(float)j-(a-f_i_j)/(f_i_jm1-f_i_j));
										}
										else
										{
											if (((f_im1_j<=a)&&(a<f_i_j))||
												((f_im1_j>=a)&&(a>f_i_j)))
											{
												XPSDrawLineFloat(display,
													drawing->pixel_map,graphics_context,
													(float)i-(a-f_i_jm1)/
													(f_im1_jm1-f_i_jm1),(float)(j-1),
													(float)i-(a-f_i_j)/
													(f_im1_j-f_i_j),(float)j);
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
											XPSDrawLineFloat(display,
												drawing->pixel_map,graphics_context,
												(float)(i-1),(float)j-(a-f_im1_j)/
												(f_im1_jm1-f_im1_j),(float)i,
												(float)j-(a-f_i_j)/(f_i_jm1-f_i_j));
										}
										else
										{
											if (((f_im1_j<=a)&&(a<f_i_j))||
												((f_im1_j>=a)&&(a>f_i_j)))
											{
												XPSDrawLineFloat(display,
													drawing->pixel_map,graphics_context,
													(float)(i-1),(float)j-(a-f_im1_j)/
													(f_im1_jm1-f_im1_j),(float)i-
													(a-f_i_j)/(f_im1_j-f_i_j),(float)j);
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
											XPSDrawLineFloat(display,
												drawing->pixel_map,
												graphics_context,(float)i-(a-f_i_j)/
												(f_im1_j-f_i_j),(float)j,(float)i,
												(float)j-(a-f_i_j)/(f_i_jm1-f_i_j));
										}
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
								graphics_context,i-1,j,i,j-1);
						} break;
						case 2: case 4:
						{
							XPSDrawLine(display,drawing->pixel_map,
								graphics_context,i-1,j-1,i,j);
						} break;
						case 3:
						{
							XPSDrawLine(display,drawing->pixel_map,
								graphics_context,i-1,j-1,i-1,j);
						} break;
						case 5:
						{
							XPSDrawLine(display,drawing->pixel_map,
								graphics_context,i-1,j-1,i,j-1);
						} break;
						case 10:
						{
							XPSDrawLine(display,drawing->pixel_map,
								graphics_context,i-1,j,i,j);
						} break;
						case 12:
						{
							XPSDrawLine(display,drawing->pixel_map,
								graphics_context,i,j-1,i,j);
						} break;
					}
				}
			}
		}
		busy_cursor_off((Widget)NULL,
			drawing_information->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_constant_thickness_contours invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_constant_thickness_contours */

static int set_sub_map_min_max(struct Sub_map *sub_map, float max_f,
	float min_f,int minimum_x,int maximum_x,int minimum_y,int maximum_y)
/*******************************************************************************
LAST MODIFIED : 18 July 2001

DESCRIPTION :
<frame>'s minimum and maximum.
*******************************************************************************/
{
	int return_code;
	struct Map_frame *frame;

	ENTER(set_sub_map_min_max);
	if (sub_map)
	{
		frame=&(sub_map->frame);
		return_code=1;
		if (max_f<min_f)
		{
			sub_map->maximum_value=0;
			sub_map->minimum_value=0;
			frame->maximum_screen_x= -1;
			frame->maximum_screen_y= -1;
			frame->minimum_screen_x= -1;
			frame->minimum_screen_y= -1;
		}/* if (max_f<min_f) */
		else
		{
			sub_map->maximum_value=max_f;
			sub_map->minimum_value=min_f;
			frame->maximum_screen_x=maximum_x;
			frame->maximum_screen_y=maximum_y;
			frame->minimum_screen_x=minimum_x;
			frame->minimum_screen_y=minimum_y;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_sub_map_min_max invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_sub_map_min_max */

static int set_map_2d_map_min_max(struct Map *map)
/*******************************************************************************
LAST MODIFIED : 18 September 2001

DESCRIPTION :
Sets the  <map>'s minimum and maximum,
based upon its  sub_map(s)'  min and max.
*******************************************************************************/
{
	float min_f,max_f;
	int i;
	struct Sub_map *sub_map;
	int return_code;

	ENTER(set_map_2d_map_min_max);
	if (map&&map->sub_map&&(sub_map=map->sub_map[0]))
	{
		return_code=1;
		if (!(map->fixed_range)||(map->minimum_value>map->maximum_value))
		{
			min_f=sub_map->minimum_value;
			max_f=sub_map->maximum_value;
			for (i=0;i<map->number_of_sub_maps;i++)
			{
				sub_map=map->sub_map[i];
				if (sub_map->minimum_value<min_f)
				{
					min_f=sub_map->minimum_value;
				}
				if (sub_map->maximum_value>max_f)
				{
					max_f=sub_map->maximum_value;
				}
			}
			map->minimum_value=min_f;
			map->maximum_value=max_f;
			map->contour_minimum=min_f;
			map->contour_maximum=max_f;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_map_2d_map_min_max invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_2d_map_min_max */

static int set_map_2d_no_interpolation_min_max(struct Map *map,
	struct Sub_map *sub_map)
/*******************************************************************************
LAST MODIFIED : 18 September 2001

DESCRIPTION :
Sets the  <map>'s minimum and maximum,
for the case map->interpolation_type==NO_INTERPOLATION.
*******************************************************************************/
{
	char undecided_accepted;
	float *electrode_value,f_approx,max_f,min_f;
	int *draw_region_number,i,maximum_x,maximum_y,minimum_x,minimum_y,
		number_of_electrodes,number_of_regions,region_number,return_code,
		*screen_x,*screen_y,temp_region_number;
	struct Device	**electrode;
	struct Map_frame *frame;
	struct Region *maximum_region,*minimum_region,*current_region;
	struct Region_list_item *region_item;
	struct Rig *rig;
	struct Signal *signal;

	ENTER(set_map_2d_no_interpolation_min_max);
	frame=(struct Map_frame *)NULL;
	region_item=(struct Region_list_item *)NULL;
	rig=(struct Rig *)NULL;
	electrode=(struct Device **)NULL;
	screen_x=(int *)NULL;
	screen_y=(int *)NULL;
	electrode_value=(float *)NULL;
	draw_region_number=(int *)NULL;
	signal=(struct Signal *)NULL;
	if (map&&sub_map)
	{
		return_code=1;
		undecided_accepted=map->undecided_accepted;
		draw_region_number=map->draw_region_number;
		number_of_electrodes=map->number_of_electrodes;
		rig=*(map->rig_pointer);
		number_of_regions=rig->number_of_regions;
		/*???DB.  Put the frame loop on the inside ? */
		/*???DB.  Electrode values are not changing with frame */
		frame=&(sub_map->frame);
		min_f=1;
		max_f=0;
		maximum_region=(struct Region *)NULL;
		minimum_region=(struct Region *)NULL;
		/* for each region */
		region_item=get_Rig_region_list(rig);
		for (temp_region_number=0;temp_region_number<number_of_regions;
				temp_region_number++)
		{
			region_number=draw_region_number[temp_region_number];
			if (0<=region_number)
			{
				current_region=get_Region_list_item_region(region_item);
				/* find maximum and minimum electrodes for region */
				electrode=map->electrodes;
				screen_x=sub_map->electrode_x;
				screen_y=sub_map->electrode_y;
				electrode_value=sub_map->electrode_value;
				for (i=number_of_electrodes;i>0;i--)
				{
					if (current_region==(*electrode)->description->region)
					{
						if ((signal=(*electrode)->signal)&&((ACCEPTED==signal->status)||
							(undecided_accepted&&(UNDECIDED==signal->status))))
						{
							f_approx= *electrode_value;
							if (max_f<min_f)
							{
								min_f=f_approx;
								max_f=f_approx;
								maximum_region=current_region;
								minimum_region=current_region;
								maximum_x= *screen_x;
								maximum_y= *screen_y;
								minimum_x=maximum_x;
								minimum_y=minimum_y;
							}
							else
							{
								if (f_approx<min_f)
								{
									min_f=f_approx;
									minimum_region=current_region;
									minimum_x= *screen_x;
									minimum_y= *screen_y;
								}
								else
								{
									if (f_approx>max_f)
									{
										max_f=f_approx;
										maximum_region=current_region;
										maximum_x= *screen_x;
										maximum_y= *screen_y;
									}
								}
							}
						}/* if ((signal=(*electrode)->signal) */
					}/* if (current_region==*/
					electrode++;
					screen_x++;
					screen_y++;
					electrode_value++;
				}
			}
			region_item=get_Region_list_item_next(region_item);
		}
		frame->maximum_region=maximum_region;
		frame->minimum_region=minimum_region;
		set_sub_map_min_max(sub_map,max_f,min_f,minimum_x,maximum_x,
			minimum_y,maximum_y);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_map_2d_no_interpolation_min_max invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_2d_no_interpolation_min_max */

static int draw_2d_contour_values(struct Map *map,struct Sub_map *sub_map,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 17 September 2001

DESCRIPTION :
Write the values of the contours.
Works off map min and max values, not sub_map.
*******************************************************************************/
{
	char draw_contour_value,value_string[11];
	Display *display;
	GC graphics_context;
	float a,contour_maximum,contour_minimum,maximum_value,minimum_value,
		string_length;
	int ascent,cell_range,cell_number,contour_areas_in_x,descent,direction,end,
		i,j,number_of_contours,number_of_contour_areas,map_x_offset,map_y_offset,
		number_of_spectrum_colours,pixel_left,pixel_top,return_code,start,temp_int,
		x_offset,x_pixel,x_separation,y_offset,y_pixel,y_separation;
	short int *contour_x,*contour_y;
	struct Map_frame *frame;
	struct Map_drawing_information *drawing_information;
	XCharStruct bounds;
	XFontStruct *font;

	ENTER(draw_2d_contour_values);
	contour_x=(short int *)NULL;
	contour_y=(short int *)NULL;
	font=(XFontStruct *)NULL;
	display=(Display *)NULL;
	frame=(struct Map_frame *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	if (map&&sub_map&&drawing&&(drawing->user_interface)&&
		(display=User_interface_get_display(drawing->user_interface))
		&&(drawing->pixel_map)&&(drawing_information=map->drawing_information))
	{
		return_code=1;
		map_x_offset=sub_map->x_offset;
		map_y_offset=sub_map->y_offset;
		font=drawing_information->font;
		if (frame=&(sub_map->frame))
		{
			if ((frame->contour_x)&&(frame->contour_y))
			{
				number_of_spectrum_colours=drawing_information->number_of_spectrum_colours;
				number_of_contour_areas=map->number_of_contour_areas;
				contour_areas_in_x=map->number_of_contour_areas_in_x;
				minimum_value=map->minimum_value;
				maximum_value=map->maximum_value;
				contour_minimum=map->contour_minimum;
				contour_maximum=map->contour_maximum;
				graphics_context=(drawing_information->graphics_context).
					contour_colour;
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
					a=(contour_maximum*(float)i+contour_minimum*
						(float)(number_of_contours-i-1))/
						(float)(number_of_contours-1);
					if (fabs(a)<
						0.00001*(fabs(contour_maximum)+fabs(contour_minimum)))
					{
						a=0;
					}
					sprintf(value_string,"%.4g",a);
					string_length=strlen(value_string);
					XTextExtents(font,value_string,string_length,&direction,
						&ascent,&descent,&bounds);
					x_offset=(bounds.lbearing-bounds.rbearing)/2;
					y_offset=(bounds.ascent-bounds.descent)/2;
					x_separation=(bounds.lbearing+bounds.rbearing);
					y_separation=(bounds.ascent+bounds.descent);
					contour_x=
						(frame->contour_x)+(cell_number*number_of_contour_areas);
					contour_y=
						(frame->contour_y)+(cell_number*number_of_contour_areas);
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
									((pixel_top=
										*(contour_x-(contour_areas_in_x+1)))>=0))
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
									graphics_context,x_pixel+x_offset+map_x_offset,y_pixel+
									y_offset+map_y_offset,value_string,string_length);
							}
						}
						contour_x++;
						contour_y++;
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"draw_2d_contour_values.  Invalid frames");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_2d_contour_values invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_contour_values */

static int draw_2d_fibres(struct Map *map,struct Sub_map *sub_map,
	struct Drawing_2d *drawing,int screen_region_width, int screen_region_height)
/*******************************************************************************
LAST MODIFIED :10 September 2001

DESCRIPTION :
Draw the fibres
*******************************************************************************/
{
	char valid_mu_and_theta;
	Display *display;
	GC graphics_context;
	float a,b,c,cos_mu_hat,cos_theta_hat,d,det,dxdmu,dxdtheta,dydmu,dydtheta,
		error_mu,error_theta,fibre_angle,fibre_angle_1,fibre_angle_2,fibre_angle_3,
		fibre_angle_4,fibre_x,fibre_y,fibre_length,*min_x,*min_y,mu,mu_1,mu_2,mu_3,mu_4,pi,
		pi_over_2,sin_mu_hat,sin_theta_hat,*stretch_x,*stretch_y,theta,theta_1,
		theta_2,theta_3,theta_4,two_pi,xi_1,xi_2,x_screen,x_screen_left,x_screen_step,
		y_screen,y_screen_top,y_screen_step;
	int *draw_region_number,fibre_iteration,fibre_spacing,i,j,k,l,map_x_offset,
		map_y_offset,map_width,map_height,number_of_regions,
		pixel_left,pixel_top,region_number,return_code,*start_x,*start_y,
		temp_region_number,x_pixel,y_pixel;
	struct Fibre_node *local_fibre_node_1,*local_fibre_node_2,*local_fibre_node_3,
		*local_fibre_node_4;
	struct Map_drawing_information *drawing_information;
	struct Region *current_region;
	struct Region_list_item *region_item;
	struct Rig *rig;

	ENTER(draw_2d_fibres);
	local_fibre_node_1=(struct Fibre_node *)NULL;
	local_fibre_node_2=(struct Fibre_node *)NULL;
	local_fibre_node_3=(struct Fibre_node *)NULL;
	local_fibre_node_4=(struct Fibre_node *)NULL;
	display=(Display *)NULL;
	region_item=(struct Region_list_item *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	if (map&&sub_map&&drawing&&(drawing->user_interface)&&(display=User_interface_get_display(drawing->user_interface))
		&&(drawing->pixel_map)&&(drawing_information=map->drawing_information)
		&&(rig=*(map->rig_pointer)))
	{
		draw_region_number=map->draw_region_number;
		map_x_offset=sub_map->x_offset;
		map_y_offset=sub_map->y_offset;
		map_width=sub_map->width;
		map_height=sub_map->height;
		start_x=sub_map->start_x;
		start_y=sub_map->start_y;
		min_x=sub_map->min_x;
		min_y=sub_map->min_y;
		stretch_x=sub_map->stretch_x;
		stretch_y=sub_map->stretch_y;
		pi_over_2=2*atan(1);
		pi=2*pi_over_2;
		two_pi=2*pi;
		number_of_regions=rig->number_of_regions;
		return_code=1;
		/* set the colour for the fibres */
		graphics_context=(drawing_information->graphics_context).spectrum,
			XSetForeground(display,graphics_context,
				drawing_information->fibre_colour);
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
		region_item=get_Rig_region_list(rig);
		for (temp_region_number=0;temp_region_number<number_of_regions;
				temp_region_number++)
		{
			region_number=draw_region_number[temp_region_number];
			if (0<=region_number)
			{
				current_region=get_Region_list_item_region(region_item);
				if ((SOCK==current_region->type)&&
					(0!=stretch_x[region_number])&&
					(0!=stretch_y[region_number]))
				{
					/* draw fibres */
					pixel_left=((region_number/map->number_of_region_rows)*
						map_width)/map->number_of_region_columns+fibre_spacing/2;
					pixel_top=((region_number%map->number_of_region_rows)*
						map_height)/map->number_of_region_rows+fibre_spacing/2;
					x_screen_step=(float)fibre_spacing/stretch_x[region_number];
					y_screen_step=
						-(float)fibre_spacing/stretch_y[region_number];
					x_screen_left=min_x[region_number]+
						(pixel_left-start_x[region_number])/
						stretch_x[region_number];
					y_screen_top=min_y[region_number]-
						(pixel_top-start_y[region_number])/
						stretch_y[region_number];
					y_screen=y_screen_top;
					y_pixel=pixel_top;
					for (j=screen_region_height/fibre_spacing;j>0;j--)
					{
						x_screen=x_screen_left;
						x_pixel=pixel_left;
						for (i=screen_region_width/fibre_spacing;i>0;i--)
						{
							/* calculate the element coordinates and the Jacobian
								for the transformation from element coordinates to
								physical coordinates */
							switch (map->projection_type)
							{
								case HAMMER_PROJECTION:
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
								case POLAR_PROJECTION:
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
								/* calculate the angle between the fibre direction and
									the positive theta direction */
								/*???DB.  Eventually this will form part of the
									"cardiac database" */
								/* the fibre direction has been fitted with a
									bilinear*/
								/* determine which element the point is in */
								local_fibre_node_1=global_fibre_nodes+
									(NUMBER_OF_FIBRE_COLUMNS-1);
								local_fibre_node_2=global_fibre_nodes;
								local_fibre_node_3=local_fibre_node_1+
									NUMBER_OF_FIBRE_COLUMNS;
								local_fibre_node_4=local_fibre_node_2+
									NUMBER_OF_FIBRE_COLUMNS;
								k=NUMBER_OF_FIBRE_ROWS;
								xi_1= -1;
								xi_2= -1;
								while ((k>0)&&
									((xi_2<0)||(xi_2>1)||(xi_1<0)||(xi_1>1)))
								{
									l=NUMBER_OF_FIBRE_COLUMNS;
									while ((l>0)&&
										((xi_2<0)||(xi_2>1)||(xi_1<0)||(xi_1>1)))
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
									/* calculate the fibre vector in element
										coordinates*/
									a=cos(fibre_angle);
									c=sin(fibre_angle);
									/* perform projection and screen scaling */
									fibre_x=stretch_x[region_number]*
										(dxdmu*c+dxdtheta*a);
									fibre_y= -stretch_y[region_number]*
										(dydmu*c+dydtheta*a);
									if (0<(fibre_length=
										fibre_x*fibre_x+fibre_y*fibre_y))
									{
										/* draw the fibre */
										fibre_length=
											(float)(fibre_spacing)/(2*sqrt(fibre_length));
										fibre_x *= fibre_length;
										fibre_y *= fibre_length;
										XPSDrawLine(display,drawing->pixel_map,
											graphics_context,
											(x_pixel-(short)fibre_x)+map_x_offset,
											(y_pixel-(short)fibre_y)+map_y_offset,
											(x_pixel+(short)fibre_x)+map_x_offset,
											(y_pixel+(short)fibre_y)+map_y_offset);
									}
									/*???debug */
									/*printf("fibre_x=%g, fibre_y=%g\n\n",fibre_x,
										fibre_y);*/
								}
							}
							x_screen += x_screen_step;
							x_pixel += fibre_spacing;
						}
						y_screen += y_screen_step;
						y_pixel += fibre_spacing;
					}
				}
			}
			region_item=get_Region_list_item_next(region_item);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_2d_fibres invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_fibres */

static int confine_text_to_map(int *x_string,int *y_string,XCharStruct *bounds,
	int ascent,int descent,int map_x_offset,int map_y_offset,int map_width,
	int map_height)
/*******************************************************************************
LAST MODIFIED : 16 July 2001

DESCRIPTION :
Limits the string position given by <x_string> <y_string>, <bounds> <ascent>,
<descent> to the map given by <map_x_offset> <map_y_offset> <map_width>
< map_height>
*******************************************************************************/
{
	int return_code;

	ENTER(confine_text_to_map);
	if (x_string&&y_string&&bounds)
	{
		return_code=1;
		if (*x_string-(*bounds).lbearing<(0+map_x_offset))
		{
			*x_string=(*bounds).lbearing+map_x_offset;
		}
		else
		{
			if (*x_string+(*bounds).rbearing>(map_width+map_x_offset))
			{
				*x_string=(map_width+map_x_offset)-(*bounds).rbearing;
			}
		}
		if (*y_string-ascent<(0+map_y_offset))
		{
			*y_string=ascent+map_y_offset;
		}
		else
		{
			if (*y_string+descent>(map_height+map_y_offset))
			{
				*y_string=(map_height+map_y_offset)-descent;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"confine_text_to_map. invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* confine_text_to_map */

static int draw_2d_extrema(struct Map *map,struct Sub_map *sub_map,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 17 September 2001

DESCRIPTION :
draw the extrema
*******************************************************************************/
{
	Display *display;
	char *name,value_string[11];
	GC graphics_context;
	int ascent,descent,direction,*draw_region_number,map_width,map_height,
		map_x_offset,map_y_offset,name_length,number_of_regions,region_number,
		return_code,temp_region_number,x_string,y_string;
	struct Map_drawing_information *drawing_information;
	struct Map_frame *frame=(struct Map_frame *)NULL;
	struct Region *current_region;
	struct Region_list_item *region_item;
	struct Rig *rig;
	XFontStruct *font;
	XCharStruct bounds;

	ENTER(draw_2d_extrema);
	name=(char *)NULL;
	display=(Display *)NULL;
	region_item=(struct Region_list_item *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	if (map&&sub_map&&drawing&&(drawing->user_interface)&&
		(display=User_interface_get_display(drawing->user_interface))
		&&(drawing->pixel_map)&&(drawing_information=map->drawing_information)
		&&(rig=*(map->rig_pointer)))
	{
	map_x_offset=sub_map->x_offset;
	map_y_offset=sub_map->y_offset;
	map_width=sub_map->width;
	map_height=sub_map->height;
	draw_region_number=map->draw_region_number;
	font=drawing_information->font;
	number_of_regions=rig->number_of_regions;
	if (frame=&(sub_map->frame))
	{
		return_code=1;
		/* set the colour for the extrema */
		graphics_context=(drawing_information->graphics_context).spectrum,
		XSetForeground(display,graphics_context,drawing_information->landmark_colour);
		region_item=get_Rig_region_list(rig);
		for (temp_region_number=0;temp_region_number<number_of_regions;
			temp_region_number++)
		{
			region_number=draw_region_number[temp_region_number];
			if (0<=region_number)
			{
				current_region=get_Region_list_item_region(region_item);
				if (frame->maximum_region==current_region)
				{
					/* draw plus */
					XPSFillRectangle(display,drawing->pixel_map,graphics_context,
						(frame->maximum_screen_x-5)+map_x_offset,(frame->maximum_screen_y-1)+map_y_offset,
						11,3);
					XPSFillRectangle(display,drawing->pixel_map,graphics_context,
						(frame->maximum_screen_x-1)+map_x_offset,(frame->maximum_screen_y-5)+map_y_offset,
						3,11);
					/* if not animation */
					if (map->activation_front<0)
					{
						/* write value */
						sprintf(value_string,"%.4g",sub_map->maximum_value);
						name=value_string;
						name_length=strlen(value_string);
						x_string=frame->maximum_screen_x;
#if defined (NO_ALIGNMENT)
						XTextExtents(font,name,name_length,&direction,&ascent,&descent,&bounds);
						x_string += (bounds.lbearing-bounds.rbearing+1)/2+map_x_offset;
#else /* defined (NO_ALIGNMENT) */
						SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
#endif /* defined (NO_ALIGNMENT) */
						if ((frame->maximum_screen_y+map_y_offset)>(map_height/2 +map_y_offset))
						{
							y_string=frame->maximum_screen_y-6 +map_y_offset;
#if defined (NO_ALIGNMENT)
							y_string -= descent+1;
#else /* defined (NO_ALIGNMENT) */
							SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
#endif /* defined (NO_ALIGNMENT) */
						}
						else
						{
							y_string=frame->maximum_screen_y+6 +map_y_offset;
#if defined (NO_ALIGNMENT)
							y_string += ascent+1;
#else /* defined (NO_ALIGNMENT) */
							SET_VERTICAL_ALIGNMENT(TOP_ALIGNMENT);
#endif /* defined (NO_ALIGNMENT) */
						}
#if defined (NO_ALIGNMENT)
/* make sure that the string doesn't extend outside the
	window */
						confine_text_to_map(&x_string,&y_string,&bounds,ascent,
							descent,map_x_offset,map_y_offset,map_width,map_height);
#endif /* defined (NO_ALIGNMENT) */
						XPSDrawString(display,drawing->pixel_map,graphics_context,x_string,
							y_string,name,name_length);
					}
				}
				if (frame->minimum_region==current_region)
				{
					/* draw minus */
					XPSFillRectangle(display,drawing->pixel_map,graphics_context,
						(frame->minimum_screen_x-5)+map_x_offset,(frame->minimum_screen_y-1)+map_y_offset,
						11,3);
					/* if not animation */
					if (map->activation_front<0)
					{
						/* write value */
						sprintf(value_string,"%.4g",sub_map->minimum_value);
						name=value_string;
						name_length=strlen(value_string);
						x_string=frame->minimum_screen_x;
#if defined (NO_ALIGNMENT)
						XTextExtents(font,name,name_length,&direction,&ascent,&descent,
							&bounds);
						x_string += (bounds.lbearing-bounds.rbearing+1)/2+map_x_offset;
#else /* defined (NO_ALIGNMENT) */
						SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
#endif /* defined (NO_ALIGNMENT) */
						if (frame->minimum_screen_y+map_y_offset>map_height/2+map_y_offset)
						{
							y_string=frame->minimum_screen_y-6 +map_y_offset;
#if defined (NO_ALIGNMENT)
							y_string -= descent+1;
#else /* defined (NO_ALIGNMENT) */
							SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
#endif /* defined (NO_ALIGNMENT) */
						}
						else
						{
							y_string=frame->minimum_screen_y+6 +map_y_offset;
#if defined (NO_ALIGNMENT)
							y_string += ascent+1;
#else /* defined (NO_ALIGNMENT) */
							SET_VERTICAL_ALIGNMENT(TOP_ALIGNMENT);
#endif /* defined (NO_ALIGNMENT) */
						}
#if defined (NO_ALIGNMENT)
						/* make sure that the string doesn't extend outside the window */
						confine_text_to_map(&x_string,&y_string,&bounds,ascent,
							descent,map_x_offset,map_y_offset,map_width,map_height);
#endif /* defined (NO_ALIGNMENT) */
						XPSDrawString(display,drawing->pixel_map,graphics_context,x_string,
							y_string,name,name_length);
					}
				}
			}
			region_item=get_Region_list_item_next(region_item);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_map_extrema.  Invalid frames");
	}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_2d_extrema invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_extrema */

static int draw_2d_landmarks(struct Map *map,struct Sub_map *sub_map,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
draw the landmarks
*******************************************************************************/
{
	Display *display;
	GC graphics_context;
	float lambda,*landmark_point,*max_x,*max_y,map_x_offset,map_y_offset,
		*min_x,*min_y,mu,*stretch_x,*stretch_y,theta,x_screen,y_screen;
	int *draw_region_number,i,number_of_regions,region_number,return_code,
		*start_x,*start_y,temp_region_number,x_pixel,y_pixel;
	struct Map_drawing_information *drawing_information;
	struct Region *current_region;
	struct Region_list_item *region_item;
	struct Rig *rig;

	ENTER(draw_2d_landmarks);
	display=(Display *)NULL;
	landmark_point=(float *)NULL;
	region_item=(struct Region_list_item *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	if (map&&sub_map&&drawing&&(drawing->user_interface)&&
		(display=User_interface_get_display(drawing->user_interface))
		&&(drawing->pixel_map)&&(drawing_information=map->drawing_information)
		&&(rig=*(map->rig_pointer)))
	{
		return_code=1;
		map_x_offset=sub_map->x_offset;
		map_y_offset=sub_map->y_offset;
		draw_region_number=map->draw_region_number;
		start_x=sub_map->start_x;
		start_y=sub_map->start_y;
		max_x=sub_map->max_x;
		max_y=sub_map->max_y;
		min_x=sub_map->min_x;
		min_y=sub_map->min_y;
		stretch_x=sub_map->stretch_x;
		stretch_y=sub_map->stretch_y;
		number_of_regions=rig->number_of_regions;
		/* set the colour for the landmarks */
		graphics_context=(drawing_information->graphics_context).spectrum,
			XSetForeground(display,graphics_context,
				drawing_information->landmark_colour);
		region_item=get_Rig_region_list(rig);
		for (temp_region_number=0;temp_region_number<number_of_regions;
				temp_region_number++)
		{
			region_number=draw_region_number[temp_region_number];
			if (0<=region_number)
			{
				current_region=get_Region_list_item_region(region_item);
				switch (current_region->type)
				{
					case SOCK:
					{
						landmark_point=landmark_points;
						for (i=NUMBER_OF_LANDMARK_POINTS;i>0;i--)
						{
							cartesian_to_prolate_spheroidal(landmark_point[0],
								landmark_point[1],landmark_point[2],LANDMARK_FOCUS,
								&lambda,&mu,&theta,(float *)NULL);
							switch (map->projection_type)
							{
								case HAMMER_PROJECTION:
								{
									Hammer_projection(mu,theta,&x_screen,&y_screen,
										(float *)NULL);
								} break;
								case POLAR_PROJECTION:
								{
									polar_projection(mu,theta,&x_screen,&y_screen,
										(float *)NULL);
								} break;
							}
							x_pixel=start_x[region_number]+
								(int)((x_screen-min_x[region_number])*
									stretch_x[region_number]);
							y_pixel=start_y[region_number]-
								(int)((y_screen-min_y[region_number])*
									stretch_y[region_number]);
							/* draw asterisk */
							XPSDrawLine(display,drawing->pixel_map,graphics_context,
								x_pixel-2+map_x_offset,y_pixel+map_y_offset,
								x_pixel+2+map_x_offset,y_pixel+map_y_offset);
							XPSDrawLine(display,drawing->pixel_map,graphics_context,
								x_pixel+map_x_offset,y_pixel-2+map_y_offset,
								x_pixel+map_x_offset,y_pixel+2+map_y_offset);
							XPSDrawLine(display,drawing->pixel_map,graphics_context,
								x_pixel-2+map_x_offset,y_pixel-2+map_y_offset,
								x_pixel+map_x_offset+2,y_pixel+2+map_y_offset);
							XPSDrawLine(display,drawing->pixel_map,graphics_context,
								x_pixel+2+map_x_offset,y_pixel+2+map_y_offset,
								x_pixel+map_x_offset-2,y_pixel-2+map_y_offset);
							landmark_point += 3;
						}
					} break;
					case TORSO:
					{
						/* draw boundary between front and back */
						x_pixel=start_x[region_number]+
							(int)((max_x[region_number]-min_x[region_number])*0.5*
								stretch_x[region_number]);
						XPSDrawLine(display,drawing->pixel_map,graphics_context,
							x_pixel+map_x_offset,
							start_y[region_number]+map_y_offset,
							x_pixel+map_x_offset,
							start_y[region_number]-(int)((max_y[region_number]-
								min_y[region_number])*stretch_y[region_number])+map_y_offset);
						x_pixel=start_x[region_number];
						XPSDrawLine(display,drawing->pixel_map,graphics_context,
							x_pixel+map_x_offset,
							start_y[region_number]+map_y_offset,
							x_pixel+map_x_offset,
							start_y[region_number]-(int)((max_y[region_number]-
								min_y[region_number])*stretch_y[region_number])+map_y_offset);
						/* draw shoulders */
						y_pixel=start_y[region_number]-
							(int)((max_y[region_number]-min_y[region_number])*
								stretch_y[region_number]);
						XPSDrawLine(display,drawing->pixel_map,graphics_context,
							start_x[region_number]+map_x_offset,
							y_pixel+map_y_offset,
							start_x[region_number]+(int)((max_x[region_number]-
								min_x[region_number])*0.1875*stretch_x[region_number])+map_x_offset,
							y_pixel-5+map_y_offset);
						XPSDrawLine(display,drawing->pixel_map,graphics_context,
							start_x[region_number]+(int)((max_x[region_number]-
								min_x[region_number])*0.5*stretch_x[region_number])+map_x_offset,
							y_pixel+map_y_offset,
							start_x[region_number]+(int)((max_x[region_number]-
								min_x[region_number])*0.3125*stretch_x[region_number])+map_x_offset,
							y_pixel-5+map_y_offset);
					} break;
				}
			}
			region_item=get_Region_list_item_next(region_item);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_2d_landmarks invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_landmarks */

static int write_map_title(struct Map *map,struct Sub_map *sub_map,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 17 October 2001

DESCRIPTION :
Write the title,using the sub_map->potential_time
*******************************************************************************/
{
	char title[12];
	float title_x,title_y,title_value;
	int ascent,descent,direction,map_width,map_height,map_x_offset,map_y_offset,
		return_code,title_length,x_string,y_string;
	struct Map_drawing_information *drawing_information;
	Display *display;
	XCharStruct bounds;
	XFontStruct *font;

	ENTER(write_map_title);
	font=(XFontStruct *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	display=(Display *)NULL;
	if (drawing&&sub_map&&(drawing->user_interface)&&
		(display=User_interface_get_display(drawing->user_interface))&&map&&
		(drawing_information=map->drawing_information))
	{
		return_code=1;
		map_width=sub_map->width;
		map_height=sub_map->height;
		map_x_offset=sub_map->x_offset;
		map_y_offset=sub_map->y_offset;
		font=drawing_information->font;
		/* write the title, using the frame time */
		title_x=map_width/2+map_x_offset;
		title_y=0;
		title_value=sub_map->frame_time;
		sprintf(title,"%.4g",title_value);
		title_length=strlen(title);
		XTextExtents(font,title,title_length,&direction,&ascent,&descent,
			&bounds);
		x_string= (title_x)+(bounds.lbearing-bounds.rbearing+1)/2;
		y_string= (title_y)-descent-1;
		confine_text_to_map(&x_string,&y_string,&bounds,ascent,descent,map_x_offset,
			map_y_offset,map_width,map_height);
		XPSDrawString(display,drawing->pixel_map,
			(drawing_information->graphics_context).
			highlighted_colour,
#if defined (NO_ALIGNMENT)
			x_string,y_string,title,title_length);
#else
		(title_x),(title_y),name,title_length);
#endif
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_map_title invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* write_map_title */

static int set_electrode_2d_name_and_colour(struct Map *map,struct Drawing_2d *drawing,
	GC *graphics_context,struct Device *electrode,char *name,float electrode_value,
	char *electrode_drawn,float range_f)
/*******************************************************************************
LAST MODIFIED : 16 July 2001

DESCRIPTION :  Set the colour (in <graphics_context>) and <name> of the electrode.
Name must already have been allocated. It's only a short string, so pass it
a static string of length 11.
*******************************************************************************/
{
	enum Map_type map_type;
	float f_value,max_f,min_f;
	int number_of_spectrum_colours,return_code;
	Display *display;
	Pixel *spectrum_pixels;
	struct Map_drawing_information *drawing_information;

	ENTER(set_electrode_2d_name_and_colour);
	spectrum_pixels=(Pixel *)NULL;
	if (map&&(drawing_information=map->drawing_information)&&graphics_context&&
		electrode&&electrode_drawn&&(display=User_interface_get_display(drawing->user_interface)))
	{
		return_code=1;
		number_of_spectrum_colours=drawing_information->number_of_spectrum_colours;
		spectrum_pixels=drawing_information->spectrum_colours;
		min_f=map->minimum_value;
		max_f=map->maximum_value;
		if (map->type)
		{
			map_type= *(map->type);
		}
		else
		{
			map_type=NO_MAP_FIELD;
		}

		if (electrode->signal->status==REJECTED)
		{
			*electrode_drawn=0;
		}
		else
		{
			*electrode_drawn=1;
		}
		switch (map->electrodes_label_type)
		{
			case HIDE_ELECTRODE_LABELS:
			{
				sprintf(name,"%s","");
				if (electrode->highlight)
				{
					*graphics_context=(drawing_information->graphics_context).
						highlighted_colour;
				}
				else
				{
					*graphics_context=(drawing_information->graphics_context).
						unhighlighted_colour;
				};
			} break;
			case SHOW_ELECTRODE_NAMES:
			case SHOW_CHANNEL_NUMBERS:
			{
				/* they're not now always drawn */
				if (SHOW_ELECTRODE_NAMES==map->electrodes_label_type)
				{
					sprintf(name,"%s",electrode->description->name);
				}
				else
				{
					sprintf(name,"%d",electrode->channel->number);
				}
			} break;
			case SHOW_ELECTRODE_VALUES:
			{
				f_value= electrode_value;
				/* if not animation */
				if (map->activation_front<0)
				{
					if (NO_MAP_FIELD==map_type)
					{
						sprintf(name,"%s","");
					}
					else
					{
						sprintf(name,"%.4g",f_value);
					}
				}
				else
				{
					sprintf(name,"%s","");
				}

			} break;
			default:
			{
				*electrode_drawn=0;
				sprintf(name,"%s","");

			} break;
		} /* switch (map->electrodes_label_type) */
		/* colour with data values*/
		if (map->colour_electrodes_with_signal)
		{
			/* electrode_drawn and electrode_value already calculated */
			f_value= electrode_value;
			switch (map_type)
			{
				case MULTIPLE_ACTIVATION:
				{
					if ((f_value<min_f)||(f_value>max_f))
					{
						*electrode_drawn=0;
					}
				} break;
			}
			if (*electrode_drawn)
			{
				/* if not animation */
				if (map->activation_front<0)
				{
					if (electrode->highlight)
					{
						*graphics_context=(drawing_information->graphics_context).
						highlighted_colour;
					}
					else
					{
						if ((HIDE_COLOUR==map->colour_option)&&
							(SHOW_CONTOURS==map->contours_option))
						{
							*graphics_context=(drawing_information->graphics_context).
								unhighlighted_colour;
						}
						else
						{
							*graphics_context=(drawing_information->graphics_context).spectrum;
							if (f_value<=min_f)
							{
								XSetForeground(display,*graphics_context,spectrum_pixels[0]);
							}
							else
							{
								if (f_value>=max_f)
								{
									XSetForeground(display,*graphics_context,spectrum_pixels
										[number_of_spectrum_colours-1]);
								}
								else
								{
									XSetForeground(display,*graphics_context,spectrum_pixels[(int)
										((f_value-min_f)*(float)(number_of_spectrum_colours-1)/
										range_f)]);
								}
							}
						}
					}
				}
				else
				{
					sprintf(name,"%s","");
					*graphics_context=(drawing_information->graphics_context).spectrum;
					if (f_value<=min_f)
					{
						XSetForeground(display,*graphics_context,spectrum_pixels[0]);
					}
					else
					{
						if (f_value>=max_f)
						{
							XSetForeground(display,*graphics_context,spectrum_pixels
							[number_of_spectrum_colours-1]);
						}
						else
						{
							XSetForeground(display,*graphics_context,spectrum_pixels[(int)
								((f_value-min_f)*(float)(number_of_spectrum_colours-1)/range_f)]);
						}
					}
				}
			}
		} /* if (map->colour_electrodes_with_signal) */
		else
		{
			if (electrode->highlight)
			{
				*graphics_context=(drawing_information->graphics_context).highlighted_colour;
			}
			else
			{
				*graphics_context=(drawing_information->graphics_context).unhighlighted_colour;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_electrode_2d_name_and_colour");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_electrode_2d_name_and_colour */

static int draw_2d_electrode(struct Map *map,struct Sub_map *sub_map,
	struct Drawing_2d *drawing,int screen_x,int screen_y,
	GC *graphics_context,char *name,struct Device *electrode)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :  Draw the electrode in 2D, and write it's name.
*******************************************************************************/
{
	Display *display;
	enum Electrodes_marker_type electrodes_marker_type;
	int ascent,descent,direction,map_x_offset,map_y_offset,map_width,map_height,
		marker_size,name_length,return_code,x_string,
		y_string;
	struct Map_drawing_information *drawing_information;
	XFontStruct *font;
	XCharStruct bounds;

	ENTER(draw_2d_electrode);
	display=(Display *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	if (map&&sub_map&&drawing&&(drawing->user_interface)&&
		(display=User_interface_get_display(drawing->user_interface))
		&&(drawing->pixel_map)&&electrode&&(drawing_information=map->drawing_information))
	{
		return_code=1;
		map_x_offset=sub_map->x_offset;
		map_y_offset=sub_map->y_offset;
		map_width=sub_map->width;
		map_height=sub_map->height;
		font=drawing_information->font;
		electrodes_marker_type=map->electrodes_marker_type;
		marker_size=map->electrodes_marker_size;
		if (marker_size<1)
		{
			marker_size=1;
		}
		switch (electrodes_marker_type)
		{
			case CIRCLE_ELECTRODE_MARKER:
			{
				/* draw circle */
				XPSFillArc(display,drawing->pixel_map,*graphics_context,
					screen_x-marker_size,screen_y-marker_size,
					2*marker_size+1,2*marker_size+1,(int)0,(int)(360*64));
			} break;
			case PLUS_ELECTRODE_MARKER:
			{
				/* draw plus */
				XPSDrawLine(display,drawing->pixel_map,*graphics_context,
					screen_x-marker_size,screen_y,screen_x+marker_size,
					screen_y);
				XPSDrawLine(display,drawing->pixel_map,*graphics_context,
					screen_x,screen_y-marker_size,screen_x,
					screen_y+marker_size);
			} break;
			case SQUARE_ELECTRODE_MARKER:
			{
				/* draw square */
				XPSFillRectangle(display,drawing->pixel_map,
					*graphics_context,screen_x-marker_size,
					screen_y-marker_size,2*marker_size+1,2*marker_size+1);
			} break;
			case HIDE_ELECTRODE_MARKER:
			{
				/* no name if hiding electrodes */
				sprintf(name,"%s","");
			} break;
		}
		if (name)
		{
			/* write name */
			name_length=strlen(name);
#if defined (NO_ALIGNMENT)
			XTextExtents(font,name,name_length,&direction,&ascent,
				&descent,&bounds);
			x_string= (screen_x)+(bounds.lbearing-bounds.rbearing+1)/2;
			y_string= (screen_y)-descent-1;
			/* make sure that the string doesn't extend outside the
				window */
			confine_text_to_map(&x_string,&y_string,&bounds,ascent,
				descent,map_x_offset,map_y_offset,map_width,map_height);
#endif
			if (electrode->highlight)
			{
				XPSDrawString(display,drawing->pixel_map,
					(drawing_information->graphics_context).
					highlighted_colour,
#if defined (NO_ALIGNMENT)
					x_string,y_string,name,name_length);
#else
				(screen_x),(screen_y)-marker_size,name,name_length);
#endif
			}
			else
			{
				XPSDrawString(display,drawing->pixel_map,
					(drawing_information->graphics_context).
					node_marker_colour,
#if defined (NO_ALIGNMENT)
					x_string,y_string,name,name_length);
#else
				(screen_x),(screen_y)-marker_size,name,name_length);
#endif
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_2d_electrode invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_electrode */

int draw_2d_show_delaunay_lines(struct Map *map,int sub_map_number,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Draws lines of delaunay triangles.
Really a debugging function.
Could be more efficient, as most lines are shared and will be drawn twice.
Must call map_2d_delaunay first.
*******************************************************************************/
{
	Display *display;
	GC graphics_context;
	int *electrode_x,*electrode_y,k,return_code,v0_index,v1_index,v2_index,x0,x1,y0,y1;
	struct Map_drawing_information *drawing_information;
	struct Sub_map *sub_map;

	ENTER(draw_2d_show_delaunay_lines);
	display=(Display *)NULL;
	sub_map=(struct Sub_map *)NULL;
	if (map&&(sub_map_number>-1)&&(sub_map_number<map->number_of_sub_maps)&&
		(sub_map=map->sub_map[sub_map_number])&&drawing&&
	(drawing_information=map->drawing_information)&&(drawing->user_interface))
	{
		return_code=1;
		display=User_interface_get_display(drawing->user_interface);
		graphics_context=(drawing_information->graphics_context).highlighted_colour;
		electrode_x=sub_map->electrode_x;
		electrode_y=sub_map->electrode_y;
		for (k=0;k<map->number_of_gouraud_triangles*3;k+=3)
		{
			v0_index=map->triangle_electrode_indices[k];
			v1_index=map->triangle_electrode_indices[k+1];
			v2_index=map->triangle_electrode_indices[k+2];
			/*first line*/
			x0=electrode_x[v0_index];
			y0=electrode_y[v0_index];
			x1=electrode_x[v1_index];
			y1=electrode_y[v1_index];
			XPSDrawLine(display,drawing->pixel_map,graphics_context,x0,y0,x1,y1);
			/*second line*/
			x0=electrode_x[v1_index];
			y0=electrode_y[v1_index];
			x1=electrode_x[v2_index];
			y1=electrode_y[v2_index];
			XPSDrawLine(display,drawing->pixel_map,graphics_context,x0,y0,x1,y1);
			/*third line*/
			x0=electrode_x[v2_index];
			y0=electrode_y[v2_index];
			x1=electrode_x[v0_index];
			y1=electrode_y[v0_index];
			XPSDrawLine(display,drawing->pixel_map,graphics_context,x0,y0,x1,y1);
		}/* (k=0;k<map->number_of_gouraud_triangles;k++) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_2d_show_delaunay_lines. invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_show_delaunay_lines */

static int draw_2d_show_map(struct Map *map,int sub_map_number,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 3 May 2002

DESCRIPTION :
Actually draw the map from the calculated data.
*******************************************************************************/
{
	char draw_boundary,draw_contours,*electrode_drawn,name[11];
	Display *display;
	enum Map_type map_type;
	float *electrode_value,max_f,min_f,*pixel_value,range_f;
	GC graphics_context;
	int map_width,map_height,map_x_offset,map_y_offset,number_of_electrodes,
		return_code,screen_region_height,screen_region_width,*screen_x,*screen_y;
	Pixel *spectrum_pixels;
	struct Device	**electrode;
	struct Map_drawing_information *drawing_information;
	struct Map_frame *frame;
	struct Sub_map *sub_map;
	XImage *frame_image;

	ENTER(draw_2d_show_map);
	spectrum_pixels=(Pixel *)NULL;
	frame=(struct Map_frame *)NULL;
	frame_image=(XImage *)NULL;
	pixel_value=(float *)NULL;
	display=(Display *)NULL;
	screen_x=(int *)NULL;
	screen_y=(int *)NULL;
	electrode=(struct Device **)NULL;
	electrode_drawn=(char *)NULL;
	electrode_value=(float *)NULL;
	sub_map=(struct Sub_map *)NULL;

	if (map&&(sub_map_number>-1)&&
		(sub_map_number<map->number_of_sub_maps)&&
		(sub_map=map->sub_map[sub_map_number])&&
		drawing&&(drawing_information=map->drawing_information)&&
		(drawing->user_interface))
	{
		return_code=1;
		map_width=sub_map->width;
		map_height=sub_map->height;
		map_x_offset=sub_map->x_offset;
		map_y_offset=sub_map->y_offset;
		screen_region_width=(map_width)/map->number_of_region_columns;
		screen_region_height=(map_height)/map->number_of_region_rows;
		map_type=*(map->type);
		spectrum_pixels=drawing_information->spectrum_colours;
		number_of_electrodes=map->number_of_electrodes;
		display=User_interface_get_display(drawing->user_interface);
		if (NO_MAP_FIELD!=map_type)
		{
			if (NO_INTERPOLATION!=map->interpolation_type)
			{
				frame=&(sub_map->frame);
				if (frame_image=frame->image)
				{
					update_colour_map_unemap(map,drawing);
					draw_2d_fill_in_image(map,sub_map,spectrum_pixels);
					if ((CONSTANT_THICKNESS==map->contour_thickness)&&
						(pixel_value=frame->pixel_values))
					{
						if (SHOW_COLOUR==map->colour_option)
						{
							XPSPutImage(display,drawing->pixel_map,
								(drawing_information->graphics_context).copy,
								frame_image,0,0,map_x_offset,map_y_offset,map_width,map_height);
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
									(drawing_information->graphics_context).copy,
									frame_image,0,0,map_x_offset,map_y_offset,map_width,
									map_height);
								draw_boundary=0;
							}
						}
						if ((SHOW_CONTOURS==map->contours_option)&&
							(map->contour_minimum<map->contour_maximum)&&
							(1<map->number_of_contours))
						{
							draw_contours=1;
						}
						else
						{
							draw_contours=0;
						}
						if (draw_contours||draw_boundary)
						{
							draw_2d_constant_thickness_contours(map,sub_map,drawing,
								pixel_value,draw_boundary);
						}
					}/* if ((CONSTANT_THICKNESS==map->contour_thickness)&&*/
					else
					{
						XPSPutImage(display,drawing->pixel_map,
							(drawing_information->graphics_context).copy,
							frame_image,0,0,map_x_offset,map_y_offset,map_width,map_height);
					}
				}/* if (frame_image=frame->image) */
			} /* if (NO_INTERPOLATION!=map->interpolation_type) */
		}/* if (NO_MAP_FIELD!=map_type) */
		/* write contour values */
		if ((HIDE_COLOUR==map->colour_option)&&
			(SHOW_CONTOURS==map->contours_option))
		{
			draw_2d_contour_values(map,sub_map,drawing);
		}
		/* draw the fibres */
		if (HIDE_FIBRES!=map->fibres_option)
		{
			draw_2d_fibres(map,sub_map,drawing,screen_region_width,
				screen_region_height);
		}
		/* draw the landmarks */
		if (SHOW_LANDMARKS==map->landmarks_option)
		{
			draw_2d_landmarks(map,sub_map,drawing);
		}

		if (map_type==POTENTIAL)
		{
			write_map_title(map,sub_map,drawing);
		}
		/* for each electrode (possibly) draw a marker at its position and its name/ */
		/* value/channel  above*/
#if !defined (NO_ALIGNMENT)
		SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
		SET_VERTICAL_ALIGNMENT(BOTTOM_ALIGNMENT);
#endif /* !defined (NO_ALIGNMENT) */
		electrode=map->electrodes;
		screen_x=sub_map->electrode_x;
		screen_y=sub_map->electrode_y;
		electrode_drawn=map->electrode_drawn;
		electrode_value=sub_map->electrode_value;
		min_f=map->minimum_value;
		max_f=map->maximum_value;
		if ((range_f=max_f-min_f)<=0)
		{
			range_f=1;
		}
		for (;number_of_electrodes>0;number_of_electrodes--)
		{
			set_electrode_2d_name_and_colour(map,drawing,&graphics_context,
			*electrode,name,*electrode_value,electrode_drawn,range_f);
			if (*electrode_drawn)
			{
				draw_2d_electrode(map,sub_map,drawing,*screen_x,*screen_y,
					&graphics_context,name,
					*electrode);
			}
			electrode++;
			screen_x++;
			screen_y++;
			electrode_drawn++;
			electrode_value++;
		}/* for (;number_of_electrodes>0;number_of_electrodes--) */
		/* draw the extrema */
		if (SHOW_EXTREMA==map->extrema_option)
		{
			draw_2d_extrema(map,sub_map,drawing);
		}
#if defined (DEBUG)
		draw_2d_show_delaunay_lines(map,sub_map_number,drawing);
#endif /* defined (DEBUG)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_show_map Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_show_map */

static int draw_2d_perform_projection_update_ranges(struct Map *map,
	struct Sub_map *sub_map,struct Device_description *description,
	float *x_item,float *y_item,char *first)
/*******************************************************************************
LAST MODIFIED : 17 July 2001

DESCRIPTION :
Perform projection and update ranges for 2d map
*******************************************************************************/
{
	float a,b,c,lambda,*max_x,*max_y,*min_x,*min_y,mu,r,theta;
	int *draw_region_number,region_number,return_code;
	struct Position *position;

	ENTER(draw_2d_perform_projection_update_ranges);
	position=(struct Position *)NULL;
	if (map&&sub_map&&description&&first)
	{
		return_code=1;
		draw_region_number=map->draw_region_number;
		max_x=sub_map->max_x;
		min_x=sub_map->min_x;
		max_y=sub_map->max_y;
		min_y=sub_map->min_y;
		/* perform projection and update ranges */
		region_number=draw_region_number[description->region->number];
		position= &(description->properties.electrode.position);
		switch (description->region->type)
		{
			case SOCK:
			{
				linear_transformation(description->region->properties.sock.
					linear_transformation,position->x,position->y,position->z,
					&a,&b,&c);
				cartesian_to_prolate_spheroidal(a,b,c,
					description->region->properties.sock.focus,&lambda,&mu,
					&theta,(float *)NULL);
				switch (map->projection_type)
				{
					case HAMMER_PROJECTION:
					{
						Hammer_projection(mu,theta,x_item,y_item,(float *)NULL);
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
					case POLAR_PROJECTION:
					{
						polar_projection(mu,theta,x_item,y_item,(float *)NULL);
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
				*x_item=position->x;
				*y_item=position->y;
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
			case TORSO:
			{
				cartesian_to_cylindrical_polar(position->x,position->y,
					position->z,&r,x_item,y_item,(float *)NULL);
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_perform_projection_update_ranges invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_perform_projection_update_ranges */

static int draw_2d_calculate_electrode_value(struct Map *map,
	struct Sub_map *sub_map,char *electrode_drawn,struct Device	**electrode,
	float *f_value)
/*******************************************************************************
LAST MODIFIED : 17 October 2001

DESCRIPTION :
Calculate electrode value for 2d <map>'s <electrode>, returns in <f_value>
*******************************************************************************/
{
	char undecided_accepted;
	double integral;
	enum Map_type map_type;
	float a,*float_value,frame_time,frame_time_freq,proportion;
	int after,before,buffer_index,datum,end_search_interval,event_number,found,i,
		middle,number_of_signals,return_code,start_search_interval,*times;
	short int	*short_int_value;
	struct Event *event;
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(draw_2d_calculate_electrode_value);
	signal=(struct Signal *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	event=(struct Event *)NULL;
	times=(int *)NULL;
	short_int_value=(short int *)NULL;
	float_value=(float *)NULL;
	if (map&&sub_map&&electrode_drawn)
	{
		frame_time=sub_map->frame_time;
		return_code=1;
		if ((map->type)&&(NO_MAP_FIELD!=(map_type= *(map->type)))&&(map->datum)&&
			(map->event_number)&&(map->end_search_interval)&&
			(map->start_search_interval))
		{
			datum= *(map->datum);
			event_number= *(map->event_number);
			undecided_accepted=map->undecided_accepted;
			end_search_interval= *(map->end_search_interval);
			start_search_interval= *(map->start_search_interval);
			switch (map_type)
			{
				case SINGLE_ACTIVATION:
				{
					if ((signal=(*electrode)->signal)&&
						((ACCEPTED==signal->status)||(undecided_accepted&&
							(UNDECIDED==signal->status)))&&
						(buffer=signal->buffer)&&(times=buffer->times))
					{
						event=signal->first_event;
						while (event&&(event->number<event_number))
						{
							event=event->next;
						}
						if (event&&(event->number==event_number)&&
							((ACCEPTED==event->status)||
								(undecided_accepted&&(UNDECIDED==event->status))))
						{
							*f_value=(float)(times[event->time]-times[datum])*1000/
								(signal->buffer->frequency);
							*electrode_drawn=1;
						}
					}
				} break;
				case MULTIPLE_ACTIVATION:
				{
					if ((signal=(*electrode)->signal)&&
						((ACCEPTED==signal->status)||(undecided_accepted&&
							(UNDECIDED==signal->status)))&&
						(buffer=signal->buffer)&&(times=buffer->times))
					{
						found=0;
						event=signal->first_event;
						while (event)
						{
							if ((ACCEPTED==event->status)||
								(undecided_accepted&&(UNDECIDED==event->status)))
							{
								a=(float)(times[datum]-times[event->time])*1000/
									(signal->buffer->frequency);
								if (found)
								{
									if (((*f_value<0)&&(*f_value<a))||
										((*f_value>=0)&&(0<=a)&&(a<*f_value)))
									{
										*f_value=a;
									}
								}
								else
								{
									found=1;
									*electrode_drawn=1;
									*f_value=a;
								}
							}
							event=event->next;
						}
					}
				} break;
				case INTEGRAL:
				{
					if ((signal=(*electrode)->signal)&&
						(0<=start_search_interval)&&
						(start_search_interval<=end_search_interval)&&
						(end_search_interval<signal->buffer->
							number_of_samples)&&
						((signal->status==ACCEPTED)||(undecided_accepted&&
							(signal->status==UNDECIDED))))
					{
						integral= -(double)((*electrode)->channel->offset)*
							(double)(end_search_interval-
								start_search_interval+1);
						number_of_signals=signal->buffer->number_of_signals;
						switch (signal->buffer->value_type)
						{
							case SHORT_INT_VALUE:
							{
								short_int_value=
									(signal->buffer->signals.short_int_values)+
									(start_search_interval*number_of_signals+
										(signal->index));
								for (i=end_search_interval-start_search_interval;
										i>=0;i--)
								{
									integral += (double)(*short_int_value);
									short_int_value += number_of_signals;
								}
							} break;
							case FLOAT_VALUE:
							{
								float_value=
									(signal->buffer->signals.float_values)+
									(start_search_interval*number_of_signals+
										(signal->index));
								for (i=end_search_interval-start_search_interval;
										i>=0;i--)
								{
									integral += (double)(*float_value);
									float_value += number_of_signals;
								}
							} break;
						}
						integral *= (double)((*electrode)->channel->gain)/
							(double)(signal->buffer->frequency);
						*electrode_drawn=1;
						*f_value=(float)integral;
					}
				} break;
				case POTENTIAL:
				{
					if (NO_INTERPOLATION==map->interpolation_type)
					{
						if ((signal=(*electrode)->signal)&&
							((ACCEPTED==signal->status)||(undecided_accepted&&
								(UNDECIDED==signal->status)))&&
							(buffer=signal->buffer)&&(times=buffer->times))
						{
							/* calculate buffer index from the frame_time */
							buffer_index=(int)(frame_time*(buffer->frequency)/1000.0)-
								(buffer->times)[0];
							if (buffer_index<0)
							{
								buffer_index=0;
							}
							if (buffer_index>=buffer->number_of_samples)
							{
								buffer_index=buffer->number_of_samples-1;
							}
							switch (buffer->value_type)
							{
								case SHORT_INT_VALUE:
								{
									*f_value=((float)((buffer->signals.short_int_values)[
										buffer_index*(buffer->number_of_signals)+(signal->index)])-
										((*electrode)->channel->offset))*
										((*electrode)->channel->gain);
								} break;
								case FLOAT_VALUE:
								{
									*f_value=((buffer->signals.float_values)[buffer_index*
										(buffer->number_of_signals)+(signal->index)]-
										((*electrode)->channel->offset))*
										((*electrode)->channel->gain);
								} break;
							}
							*electrode_drawn=1;
						}
					}
					else
					{
						frame_time=sub_map->frame_time;
						if ((signal=(*electrode)->signal)&&
							((ACCEPTED==signal->status)||(undecided_accepted&&
								(UNDECIDED==signal->status)))&&
							(buffer=signal->buffer)&&(times=buffer->times)&&
							((float)(times[0])<=(frame_time_freq=frame_time*
								(buffer->frequency)/1000))&&(frame_time_freq<=
									(float)(times[(buffer->number_of_samples)-1])))
						{
							before=0;
							after=(buffer->number_of_samples)-1;
							while (before+1<after)
							{
								middle=(before+after)/2;
								if (frame_time_freq<times[middle])
								{
									after=middle;
								}
								else
								{
									before=middle;
								}
							}
							if (before==after)
							{
								proportion=0.5;
							}
							else
							{
								proportion=((float)(times[after])-frame_time_freq)/
									(float)(times[after]-times[before]);
							}
							switch (buffer->value_type)
							{
								case SHORT_INT_VALUE:
								{
									*f_value=(proportion*(float)((buffer->signals.
										short_int_values)[before*
											(buffer->number_of_signals)+(signal->index)])+
										(1-proportion)*(float)((buffer->signals.
											short_int_values)[after*
												(buffer->number_of_signals)+(signal->index)])-
										((*electrode)->channel->offset))*
										((*electrode)->channel->gain);
								} break;
								case FLOAT_VALUE:
								{
									*f_value=(proportion*(buffer->signals.float_values)[
										before*(buffer->number_of_signals)+
										(signal->index)]+(1-proportion)*
										(buffer->signals.float_values)[after*
											(buffer->number_of_signals)+(signal->index)]-
										((*electrode)->channel->offset))*
										((*electrode)->channel->gain);
								} break;
							}
							*electrode_drawn=1;
						}
					}
				} break;
			}
		}
		else
		{
			/* always draw all the electrodes if have no map */
			*electrode_drawn=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_calculate_electrode_value arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_calculate_electrode_value */

static int draw_2d_calculate_u_and_v(struct Map *map,
	struct Region *current_region,float x_screen,float y_screen,
	float *u,float *v,char *valid_u_and_v)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
calculate the element coordinates for the 2d map.
*******************************************************************************/
{
	float a,b,c,d,mu,pi_over_2,two_pi,theta;
	int return_code;

	ENTER(draw_2d_calculate_u_and_v );
	if (map&&current_region)
	{
		pi_over_2=PI/2.0;
		two_pi=2*PI;
		return_code=1;
		/* calculate the element coordinates */
		switch (current_region->type)
		{
			case SOCK:
			{
				switch (map->projection_type)
				{
					case HAMMER_PROJECTION:
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
									mu=PI;
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
										theta=PI-2*asin(d);
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
										theta=PI+2*asin(d);
									}
								}
								*u=theta;
								*v=mu;
								*valid_u_and_v=1;
							}
							else
							{
								*valid_u_and_v=0;
							}
						}
						else
						{
							*valid_u_and_v=0;
						}
					} break;
					case POLAR_PROJECTION:
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
									theta=two_pi+
										asin(y_screen/mu);
								}
							}
							else
							{
								theta=PI-asin(y_screen/mu);
							}
						}
						else
						{
							theta=0;
						}
						*u=theta;
						*v=mu;
						*valid_u_and_v=1;
					} break;
				}/* switch (map->projection_type) */
			} break;
			case PATCH:
			{
				*u=x_screen;
				*v=y_screen;
				*valid_u_and_v=1;
			} break;
			case TORSO:
			{
				/*???DB.  Need scaling in x ? */
				*u=x_screen;
				*v=y_screen;
				*valid_u_and_v=1;
			} break;
		}/* switch (current_region->type) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_calculate_u_and_v  arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_calculate_u_and_v */

static int draw_2d_calculate_f_approx(struct Interpolation_function *function,
	float *f_approx,int column,int row,float u,float v)
/*******************************************************************************
LAST MODIFIED : 17 July 2001

DESCRIPTION :
Set <f_approx> from <function>
*******************************************************************************/
{
	float *d2fdxdy,*dfdy,*f,*dfdx,dfdx_i_j,dfdx_i_jm1,dfdx_im1_j,
		dfdx_im1_jm1,dfdy_i_j,dfdy_i_jm1,dfdy_im1_j,dfdy_im1_jm1,d2fdxdy_i_j,
		d2fdxdy_i_jm1,d2fdxdy_im1_j,d2fdxdy_im1_jm1,f_i_j,f_i_jm1,
		f_im1_j,f_im1_jm1,height,h01_u,h01_v,h02_u,h02_v,
		h11_u,h11_v,h12_u,h12_v,width,*x_mesh,*y_mesh;
	int i_j,i_jm1,im1_j,im1_jm1,number_of_mesh_columns,return_code;

	ENTER(draw_2d_calculate_f_approx);
	if (f_approx)
	{
		f=function->f;
		dfdx=function->dfdx;
		dfdy=function->dfdy;
		d2fdxdy=function->d2fdxdy;
		number_of_mesh_columns=function->number_of_columns;
		y_mesh=function->y_mesh;
		x_mesh=function->x_mesh;
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
		/* calculate the interpolation
			function coefficients */
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
			NB the local coordinates have the
			top right corner of the element as
			the origin.  This means that
			(i-1,j-1) is the bottom left corner
			(i,j-1) is the top left corner
			(i,j) is the top right corner
			(i-1,j) is the bottom right
			corner */
		/* (i-1,j-1) node (bottom left
			corner) */
		im1_jm1=(row-1)*(number_of_mesh_columns+1)+column-1;
		/* (i,j-1) node (top left corner) */
		i_jm1=row*(number_of_mesh_columns+1)+column-1;
		/* (i,j) node (top right corner) */
		i_j=row*(number_of_mesh_columns+1)+column;
		/* (i-1,j) node (bottom right
			corner) */
		im1_j=(row-1)*(number_of_mesh_columns+1)+column;
		*f_approx=
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_calculate_f_approx Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_calculate_f_approx */

static int draw_2d_map_boundary(char *background_map_boundary_base,
	int map_height,int map_width,float *pixel_value,float boundary_pixel_value,
	float background_pixel_value)
/*******************************************************************************
LAST MODIFIED : 20 July 2001

DESCRIPTION : (possibly) draw map boundary
*******************************************************************************/
{
	char *background_map_boundary;
	int i,j,return_code;

	ENTER(draw_2d_map_boundary);
	background_map_boundary=(char *)NULL;

	if (background_map_boundary_base&&pixel_value)
	{
		return_code=1;
		background_map_boundary=background_map_boundary_base;
		for (i=map_height;i>0;i--)
		{
			for (j=map_width;j>0;j--)
			{
				if (0== *background_map_boundary)
				{
					if (((i<map_height)&&(1==
						*(background_map_boundary-map_width)))||
						((i>1)&&(1==
							*(background_map_boundary+map_width)))||
						((j<map_width)&&(1==
							*(background_map_boundary-1)))||
						((j>1)&&(1== *(background_map_boundary+1))))
					{
						*pixel_value=boundary_pixel_value;
					}
					else
					{
						*pixel_value=background_pixel_value;
					}
				}
				background_map_boundary++;
				pixel_value++;
			}
		}/* for (i=map_height;i>0;i--)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_map_boundary Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* draw_2d_map_boundary */

static int draw_2d_set_min_x_max_x(struct Map *map,struct Sub_map *sub_map,
	float *a,enum Projection_type map_projection_type)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
sets <sub_map>'s min_x, max_x min_y max_y
*******************************************************************************/
{
	float *min_x,*max_x,*min_y,*max_y,pi;
	int *draw_region_number,region_number,return_code,temp_region_number,
		number_of_regions;
	struct Region_list_item *region_item;
	struct Region *region;
	struct Rig *rig;

	ENTER(draw_2d_set_min_x_max_x);
	region_item=(struct Region_list_item *)NULL;
	region=(struct Region *)NULL;
	if (map&&sub_map&&(rig= *(map->rig_pointer))&&
		(0<(number_of_regions=rig->number_of_regions)))
	{
		return_code=1;
		pi=4*atan(1);
		draw_region_number=map->draw_region_number;
		min_x=sub_map->min_x;
		min_y=sub_map->min_y;
		max_x=sub_map->max_x;
		max_y=sub_map->max_y;
		/* divide the drawing area into regions */
		region_item=get_Rig_region_list(rig);
		for (temp_region_number=0;temp_region_number<number_of_regions;
				temp_region_number++)
		{
			region_number=draw_region_number[temp_region_number];
			if (0<=region_number)
			{
				region=get_Region_list_item_region(region_item);
				switch (region->type)
				{
					case SOCK:
					{
						switch (map_projection_type)
						{
							case HAMMER_PROJECTION:
							{
								Hammer_projection(max_x[region_number],0,a,
									max_y+region_number,(float *)NULL);
								min_x[region_number]= -1;
								max_x[region_number]=1;
								min_y[region_number]= -1;
							} break;
							case POLAR_PROJECTION:
							{
								min_x[region_number]= -max_x[region_number];
								min_y[region_number]=min_x[region_number];
								max_y[region_number]=max_x[region_number];
							} break;
						}
					} break;
					case TORSO:
					{
						min_x[region_number]= -pi;
						max_x[region_number]=pi;
					} break;
				}/* switch (region->type) */
			}/* for (temp_region_number=0;temp_region_number<number_of_regions; */
			region_item=get_Region_list_item_next(region_item);
		}/* for (temp_region_number=0;temp_region_number<number_of_regions; */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_set_min_x_max_x Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_set_min_x_max_x */

static int draw_2d_calc_map_to_screen_transform(struct Map *map,
	struct Sub_map *sub_map,Display *display,int screen_region_width,
	int screen_region_height,int x_border,int y_border,int ascent,int descent)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Calculate the screen to map transform for the 2d Map.
*******************************************************************************/
{
	float *min_x,*max_x,*min_y,*max_y,*stretch_x,*stretch_y,
		pixel_aspect_ratio;
	int i,map_height,map_width,number_of_drawn_regions,return_code,*start_x,
		*start_y;

	ENTER(draw_2d_calc_map_to_screen_transform);
	if (map&&display&&sub_map)
	{
		return_code=1;
		map_height=sub_map->height;
		map_width=sub_map->width;
		number_of_drawn_regions=map->number_of_drawn_regions;
		min_x=sub_map->min_x;
		max_x=sub_map->max_x;
		min_y=sub_map->min_y;
		max_y=sub_map->max_y;
		start_x=sub_map->start_x;
		start_y=sub_map->start_y;
		stretch_x=sub_map->stretch_x;
		stretch_y=sub_map->stretch_y;
		/* calculate the transformation from map coordinates to screen coordinates */
		pixel_aspect_ratio=get_pixel_aspect_ratio(display);
		for (i=0;i<number_of_drawn_regions;i++)
		{
			if ((map->maintain_aspect_ratio)&&(max_x[i]!=min_x[i])&&
				(screen_region_width>2*x_border+1)&&(max_y[i]!=min_y[i])&&
				(screen_region_height>2*y_border+1))
			{
				if ((float)((max_y[i]-min_y[i])*(screen_region_width-
					(2*x_border+1)))<(float)((max_x[i]-min_x[i])*
						(screen_region_height-(2*y_border+ascent+descent+1)))*
					pixel_aspect_ratio)
				{
					/* fill width */
					start_x[i]=x_border;
					stretch_x[i]=((float)(screen_region_width-(2*x_border+1)))/
						(max_x[i]-min_x[i]);
					stretch_y[i]=stretch_x[i]/pixel_aspect_ratio;
					start_y[i]=(screen_region_height+
						(int)((max_y[i]-min_y[i])*stretch_y[i]))/2;
				}
				else
				{
					/* fill height */
					start_y[i]=screen_region_height-(y_border+1);
					stretch_y[i]=((float)(screen_region_height-
						(2*y_border+ascent+descent+1)))/(max_y[i]-min_y[i]);
					stretch_x[i]=stretch_y[i]*pixel_aspect_ratio;
					start_x[i]=(screen_region_width-
						(int)((max_x[i]-min_x[i])*stretch_x[i]))/2;
				}
			}
			else
			{
				if ((max_x[i]==min_x[i])||(screen_region_width<=2*x_border+1))
				{
					start_x[i]=(screen_region_width)/2;
					stretch_x[i]=0;
				}
				else
				{
					start_x[i]=x_border;
					stretch_x[i]=((float)(screen_region_width-(2*x_border+1)))/
						(max_x[i]-min_x[i]);
				}
				if ((max_y[i]==min_y[i])||(screen_region_height<=2*y_border+1))
				{
					start_y[i]=(screen_region_height)/2;
					stretch_y[i]=0;
				}
				else
				{
					start_y[i]=screen_region_height-(y_border+1);
					stretch_y[i]=((float)(screen_region_height-
						(2*y_border+ascent+descent+1)))/(max_y[i]-min_y[i]);
				}
			}
			start_x[i] += ((i/map->number_of_region_rows)*map_width)/
				map->number_of_region_columns;
			start_y[i] += ((i%map->number_of_region_rows)*map_height)/
				map->number_of_region_rows;
		}/* for (i=0;i<number_of_drawn_regions;i++) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_calc_map_to_screen_transform Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_calc_map_to_screen_transform */

static int triangle_rc_to_cp_vertex(int triangle_vertex_index,float *vertices,
	float *r,float *theta,float *z_cp)
/*******************************************************************************
LAST MODIFIED :7 November 2001

DESCRIPTION :
Given a triangle's vertex given by the offset <triangle_vertex_index> into the
array of rectangular cartesian vertices <vertices>, converts this x,y,z into
<r> <theta> <z_cp> and returns it.
See cylinder_delaunay for definition of <triangle_vertex_index>, <vertices>
*******************************************************************************/
{
	float x,y,z_rc;
	int return_code,index;

	ENTER(triangle_rc_to_cp_vertex);
	if (vertices&&r&&theta&&z_cp)
	{
		return_code=1;
		/*each vertex has 3 dimesions*/
		index=triangle_vertex_index*3;
		x=vertices[index];
		y=vertices[index+1];
		z_rc=vertices[index+2];
		cartesian_to_cylindrical_polar(x,y,z_rc,r,theta,z_cp,(float *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"triangle_rc_to_cp_vertex Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* triangle_rc_to_cp_vertex */

int triangle_cut_by_angle(int vertex0_index,int vertex1_index,
	int vertex2_index,float *vertices,float angle)
/*******************************************************************************
LAST MODIFIED :8 November 2001

DESCRIPTION :
Given the triangle defined by <vertex0_index> <vertex1_index>,<vertex2_index>,
<vertices>, converts this triangle from rectangular cartesian to cylindrical
polar coords, and determines if it's min and max theta are either side of
<angle>. Takes the minimum angle btwn the two points, i.e PI/2 not 3PI/2.
Assume all angles from -PI to PI.
See cylinder_delaunay for definition of <vertex0_index> <vertex1_index>
<vertex2_index>,<*vertices>.
Would be possible to generalise for any polygon, but this would be slower,
and I don't see a need.
*******************************************************************************/
{
	float r[3],theta[3],z_cp[3],min_theta,max_theta;
	int i,return_code;

	ENTER(triangle_cut_by_angle);
	return_code=0;
	if (vertices)
	{
		triangle_rc_to_cp_vertex(vertex0_index,vertices,&r[0],&theta[0],&z_cp[0]);
		triangle_rc_to_cp_vertex(vertex1_index,vertices,&r[1],&theta[1],&z_cp[1]);
		triangle_rc_to_cp_vertex(vertex2_index,vertices,&r[2],&theta[2],&z_cp[2]);
		/*find min and max theta of triangle*/
		min_theta=theta[0];
		max_theta=theta[0];
		for(i=0;i<3;i++)
		{
			if (theta[i]>max_theta)
			{
				max_theta=theta[i];
			}
			if (theta[i]<min_theta)
			{
				min_theta=theta[i];
			}
		}
		/*determine if angle btwin min and max theta*/
		if (fabs(max_theta-min_theta)<PI)
			/*subtended angle is the smallest; are we inside it?*/
		{
			if ((angle>min_theta)&&(angle<max_theta))
			{
				return_code=1;
			}
		}
		else
		{
			/*subtended angle is the largest; are we outside it?, i.e inside the smallest*/
			if (!((angle>min_theta)&&(angle<max_theta)))
			{
				return_code=1;
			}
		}
#if defined (DEBUG)
		if (return_code)
		{
			printf("min_theta %f Angle %f max_theta %f \n",min_theta,angle,max_theta);fflush(NULL);
		}
#endif /* defined (DEBUG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"triangle_cut_by_angle Invalid arguments");
	}
	LEAVE;
	
	return (return_code);
}/*triangle_cut_by_angle*/

static int draw_2d_get_min_max(float value,float *min_value,float *max_value,
	struct Region **maximum_region,struct Region **minimum_region,
	struct Region *region,int *minimum_x,int *minimum_y,int *maximum_x,
	int *maximum_y,int x,int y)
/*******************************************************************************
LAST MODIFIED 9 November 2001

DESCRIPTION :
Sets <min_value> <max_value> based on <value>,
	<maximum_region> <minimum_region> based on <region>
	<minimum_x> <minimum_y> <maximum_x> <maximum_y> based on <x> ,<y>.
*******************************************************************************/
{
	int return_code;

	ENTER(draw_2d_get_min_max);
	if (min_value&&max_value&&maximum_region&&minimum_region&&region&&minimum_x
		&&minimum_y&&maximum_x&&maximum_y)
	{
		return_code=1;
		if (*max_value<*min_value)
		{
			*min_value=value;
			*max_value=value;
			*maximum_region=region;
			*minimum_region=region;
			*maximum_x=x;
			*maximum_y=y;
			*minimum_x=x;
			*minimum_y=y;
		}/* if (max_value<min_value) */
		else
		{
			if (value<*min_value)
			{
				*min_value=value;
				*minimum_region=region;
				*minimum_x=x;
				*minimum_y=y;
			}
			else
			{
				if (value>*max_value)
				{
					*max_value=value;
					*maximum_region=region;
					*maximum_x=x;
					*maximum_y=y;
				}
			}
		}/* if (max_value<min_value) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_2d_get_min_max. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_get_min_max*/

static int draw_2d_construct_interpolated_image_map(struct Map *map,
	struct Sub_map *sub_map,int region_number,struct Region *the_region,
	struct Region **maximum_region,struct Region **minimum_region,
	int screen_region_height,int screen_region_width,
	int *minimum_x,int *minimum_y,int *maximum_x,int *maximum_y,
	float *max_f,float *min_f,char *background_map_boundary_base)
/*******************************************************************************
LAST MODIFIED 9 November 2001

DESCRIPTION :
Construct a colour map image for colour map or contours or  values  in the
<sub_map> frame, using interpolation.
*******************************************************************************/
{
	enum Map_type map_type;
	char valid_u_and_v;
	float frame_time,f_approx,*min_x,*min_y,*pixel_value,*stretch_x,*stretch_y,u,v,
		*x_mesh,*y_mesh,x_screen,x_screen_left,x_screen_step,y_screen,y_screen_top,
		y_screen_step;
	int column,i,j,map_width,map_height,number_of_mesh_rows,number_of_mesh_columns,
		pixel_left,pixel_top,return_code,row,*start_x,*start_y,x_pixel,y_pixel;
	struct Interpolation_function *function;
	struct Map_frame *frame;
	struct Rig *rig;

	ENTER(draw_2d_construct_interpolated_image_map);
	stretch_y=(float *)NULL;
	stretch_x=(float *)NULL;
	function=(struct Interpolation_function *)NULL;
	rig=(struct Rig *)NULL;
	x_mesh=(float *)NULL;
	y_mesh=(float *)NULL;
	min_x=(float *)NULL;
	min_y=(float *)NULL;
	start_x=(int *)NULL;
	start_y=(int *)NULL;
	pixel_value=(float *)NULL;
	frame=(struct Map_frame *)NULL;
	if (map&&sub_map&&the_region&&maximum_region&&minimum_region&&minimum_x&&
		minimum_y&&maximum_x&&maximum_y&&max_f&&min_f&&background_map_boundary_base)
	{
		map_width=sub_map->width;
		map_height=sub_map->height;
		stretch_x=sub_map->stretch_x;
		stretch_y=sub_map->stretch_y;
		min_x=sub_map->min_x;
		min_y=sub_map->min_y;
		start_x=sub_map->start_x;
		start_y=sub_map->start_y;
		if (map->type)
		{
			map_type= *(map->type);
		}
		else
		{
			map_type=NO_MAP_FIELD;
		}
		rig= *(map->rig_pointer);
		frame_time=sub_map->frame_time;
		frame=&(sub_map->frame);
		pixel_value=frame->pixel_values;
		/* interpolate data */
		if ((0!=stretch_x[region_number])&&
			(0!=stretch_y[region_number])&&
			(function=calculate_interpolation_functio(
				map_type,rig,the_region,map->event_number,
				frame_time,map->datum,
				map->start_search_interval,
				map->end_search_interval,map->undecided_accepted,
				map->finite_element_mesh_rows,
				map->finite_element_mesh_columns,
				map->membrane_smoothing,
				map->plate_bending_smoothing)))
		{
			number_of_mesh_columns=
				function->number_of_columns;
			number_of_mesh_rows=
				function->number_of_rows;
			y_mesh=function->y_mesh;
			x_mesh=function->x_mesh;
			/* calculate pixel values */
			pixel_left=((region_number/map->number_of_region_rows)*
				map_width)/map->number_of_region_columns;
			pixel_top=((region_number%map->number_of_region_rows)*
				map_height)/map->number_of_region_rows;
			x_screen_step=1/stretch_x[region_number];
			y_screen_step= -1/stretch_y[region_number];
			x_screen_left=min_x[region_number]+
				(pixel_left-start_x[region_number])*
				x_screen_step;
			y_screen_top=min_y[region_number]+
				(pixel_top-start_y[region_number])*
				y_screen_step;
			y_screen=y_screen_top;
			y_pixel=pixel_top;
			for (j=0;j<screen_region_height;j++)
			{
				x_screen=x_screen_left;
				x_pixel=pixel_left;
				for (i=0;i<screen_region_width;i++)
				{
					/* calculate the u and v  */
					draw_2d_calculate_u_and_v(map,the_region,x_screen,y_screen,&u,&v,
						&valid_u_and_v);
					/* calculate the element coordinates */
					if (valid_u_and_v)
					{
						/* determine which element the point is
							in */
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
								draw_2d_calculate_f_approx(function,
									&f_approx,column,row,u,v);
								draw_2d_get_min_max(f_approx,min_f,max_f,maximum_region,
									minimum_region,the_region,minimum_x,minimum_y,maximum_x,
									maximum_y,i,j);
								pixel_value[y_pixel*map_width+x_pixel]=f_approx;
								background_map_boundary_base[
									y_pixel*map_width+x_pixel]=1;
							}/* if ((row>0)&&(v<=y_mesh[row])) */
						}/* if ((column>0)&&(u<=x_mesh[column])) */
					}/* if (valid_u_and_v) */
					x_screen += x_screen_step;
					x_pixel++;
				}/* for (i=0;i<screen_region_width;i++) */
				y_screen += y_screen_step;
				y_pixel++;
			}/* for (j=0;j<screen_region_height;j++) */
			destroy_Interpolation_function(&function);
		}/* if ((0!=stretch_x[region_number])&& */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_construct_interpolated_image_map. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_construct_interpolated_image_map*/

static int get_triangle_scan_line_intersection(int triangle_egde_number,
	int scan_line_y,int *vertex_x,int *vertex_y,float *gradient,
	float *inv_edge_y_length,float *vertex_value,int *x_intersection,float *value)
/*******************************************************************************
LAST MODIFIED 13 November 2001

DESCRIPTION :
Given <triangle_egde_number> (from 0-2) and scan_line_y, the y value of the
intersecting "scan line" and
<vertex_x> <vertex_y> <vertex_value> arrays of length 3 of the x,y value at
the triangle's vertices and
<gradient> an array of length 3 of the gradients of the triangle's edges,
and <inv_edge_y_length> an array of length 3 of the 1/(triangle's edges y length)
returns <x_intersection>, the x value of the intersection
of the triangle edge with the "scan line" and <value>, the interpolated value
at this point (x_intersection,y).
Returns an error if scan_line_y isn't btwn the y's of the triangle edge's ends,
or if the y's of the triangle edge's end are the same, i.e you're supposed to
check for these conditions before calling this function.
This function is a quite specific (i.e. non-generic) helper function, called by
draw_2d_construct_2d_gouraud_image_map.
cf get_tri_scan_line_intersection_rgb_triple
*******************************************************************************/
{
	float grad,inverse_y_length,value_0,value_1;
	int a_x,b_x,a_y,b_y,end_vertex_num,return_code,start_vertex_num;

	ENTER(get_triangle_scan_line_intersection);
	if ((triangle_egde_number>-1)&&(triangle_egde_number<3)&&vertex_x&&vertex_y&&
		gradient&&inv_edge_y_length&&vertex_value&&x_intersection&&value)
	{
		return_code=1;
		start_vertex_num=triangle_egde_number;
		end_vertex_num=
			get_triangle_line_end_vertex_num_from_start_num(start_vertex_num);
		a_x=vertex_x[start_vertex_num];
		a_y=vertex_y[start_vertex_num];
		b_x=vertex_x[end_vertex_num];
		b_y=vertex_y[end_vertex_num];
		if (a_y!=b_y)
		{
			if (((scan_line_y>=a_y)&&(scan_line_y<=b_y))||
				((scan_line_y>=b_y)&&(scan_line_y<=a_y)))
			{
				grad=gradient[triangle_egde_number];
				inverse_y_length=inv_edge_y_length[triangle_egde_number];
				/*line_start_x or	line_end_x	*/
				if (a_x==b_x)
				{
					/*infinite gradient*/
					*x_intersection=a_x;
				}
				else
				{
					*x_intersection=(((float)scan_line_y-(float)a_y)/grad)+a_x;
				}
				/*value at start of triangle edge*/
				value_0=vertex_value[start_vertex_num];
				/*value at end of triangle edge*/
				value_1=vertex_value[end_vertex_num];
				/* inverse_y_length= 1/(b_y-ay), but we cached it earlier	*/
				*value=value_1*(((float)scan_line_y-(float)a_y)*inverse_y_length)+
					value_0*(((float)b_y-(float)scan_line_y)*inverse_y_length);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_triangle_scan_line_intersection. scan_line_y not btwn a_y and b_y ");
				return_code=0;
			}
		}/* if (a_y!=b_y) */
		else
		{
			display_message(ERROR_MESSAGE,
			"get_triangle_scan_line_intersection. ay==by ");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_triangle_scan_line_intersection. Invalid arguments");
		return_code=0;
	}
	if (return_code==0)
	{
		*value=0;
		*x_intersection=0;
	}
	LEAVE;
	
	return (return_code);
}/* get_triangle_scan_line_intersection */

static int draw_2d_construct_2d_gouraud_image_map(struct Map *map,
	struct Sub_map *sub_map,int region_number,struct Region *the_region,
	struct Region **maximum_region,struct Region **minimum_region,
	int *minimum_x,int *minimum_y,int *maximum_x,int *maximum_y,float *max_f,
	float *min_f,char *background_map_boundary_base)
/*******************************************************************************
LAST MODIFIED 26 February 2002

DESCRIPTION :
Construct a colour map image for colour map or contours or  values  in the
<sub_map> frame, using gouraud shading btwn the electrodes.
Only works for TORSOs, requires that the  electrodes are approximately a
cylinder with z axis  up centre.
cf construct_rgb_triple_gouraud_map which works with rgb triples, not floats
*******************************************************************************/
{
	enum Map_type map_type;
	float delta_value,delta_y,end_value,*electrode_value,gradient[3],
		inv_edge_y_length[3],*pixel_value,start_value,*stretch_x,*stretch_y,value,
		vertex_value[3];
	int adjusted_x,*electrode_x,*electrode_y,end_vertex_num,found,i,index,
		intersected_edges[3],k,line_start_x,line_end_x,map_width,
		number_of_intersected_edges,return_code,start_vertex_num,
		triangle_egde_number,tri_min_x,tri_min_y,tri_max_x,
		tri_max_y,tri_width,vertex_x[3],vertex_y[3],x,x_offset,y;
	struct Map_frame *frame;
#if defined (DEBUG)
	int edge_flag;
#endif 	/*  defined (DEBUG)	*/

	ENTER(draw_2d_construct_2d_gouraud_image_map);
	stretch_y=(float *)NULL;
	stretch_x=(float *)NULL;
	electrode_value=(float *)NULL;
	electrode_x=(int *)NULL;
	electrode_y=(int *)NULL;
	pixel_value=(float *)NULL;
	frame=(struct Map_frame *)NULL;
	if (map&&sub_map&&the_region&&maximum_region&&minimum_region&&minimum_x&&
		minimum_y&&maximum_x&&maximum_y&&max_f&&min_f&&background_map_boundary_base&&
		(the_region->type==TORSO))
	{
		/*if necessary, do triangulation */
		if (get_map_drawing_information_electrodes_accepted_or_rejected
			(map->drawing_information)||(!map->triangle_electrode_indices))
		{
			map_2d_delaunay(map);
			set_map_drawing_information_electrodes_accepted_or_rejected
				(map->drawing_information,0);
		}
		map_width=sub_map->width;
		stretch_x=sub_map->stretch_x;
		stretch_y=sub_map->stretch_y;
		if (map->type)
		{
			map_type= *(map->type);
		}
		else
		{
			map_type=NO_MAP_FIELD;
		}
		if (map_type==POTENTIAL)
		{
			frame=&(sub_map->frame);
			pixel_value=frame->pixel_values;
			/* x_offset is the offset into the window of the image */
			x_offset=sub_map->start_x[region_number];
			/* interpolate data */
			if ((0!=stretch_x[region_number])&&(0!=stretch_y[region_number]))
			{
				/* calculate pixel values */
				electrode_x=sub_map->electrode_x;
				electrode_y=sub_map->electrode_y;
				electrode_value=sub_map->electrode_value;
				k=0;
				while(k<map->number_of_gouraud_triangles*3)
				{
					/* for each triangle vertex*/
					for(i=0;i<3;i++)
					{
						index=map->triangle_electrode_indices[k];
						vertex_x[i]=electrode_x[index];
						vertex_y[i]=electrode_y[index];
						/*electrodes are in screen coords, but vertices independent of it */
						/* so remove their offset*/
						vertex_x[i]-=sub_map->x_offset;
						vertex_y[i]-=sub_map->y_offset;
						vertex_value[i]=electrode_value[index];
						/*get min and max x and y of triangle*/
						if (i==0)
						{
							tri_min_x=tri_max_x=vertex_x[0];
							tri_min_y=tri_max_y=vertex_y[0];
						}
						if (vertex_x[i]>tri_max_x)
						{
							tri_max_x=vertex_x[i];
						}
						if (vertex_x[i]<tri_min_x)
						{
							tri_min_x=vertex_x[i];
						}
						if (vertex_y[i]>tri_max_y)
						{
							tri_max_y=vertex_y[i];
						}
						if (vertex_y[i]<tri_min_y)
						{
							tri_min_y=vertex_y[i];
						}
						/*draw_2d_get_min_max only needs to be called for the vertices*/
						draw_2d_get_min_max(vertex_value[i],min_f,max_f,maximum_region,
							minimum_region,the_region,minimum_x,minimum_y,maximum_x,
							maximum_y,vertex_x[i],vertex_y[i]);
						k++;
					}/* for(i=0;i<3;i++)*/
					/*wide  triangles have wrapped around the back and in cp coords, crossed*/
					/*PI/-PI. Extend these wrapped triangles in rc x, so that they'll be drawn */
					/*at the edge of the map, not behind it.*/
					tri_width=abs(tri_max_x-tri_min_x);
					if (tri_width>(map_width/2))
					{
						for(i=0;i<3;i++)
						{
							if (vertex_x[i]<(map_width/2))
							{
								vertex_x[i]+=map_width-2*x_offset;
							}
						}
					}
					/*determine and cache the gradient and inverse of the y_length of the*/
					/* triangle's edges*/
					for(i=0;i<3;i++)
					{
						start_vertex_num=i;
						end_vertex_num=
							get_triangle_line_end_vertex_num_from_start_num(start_vertex_num);
						delta_y=(float)vertex_y[end_vertex_num]-(float)vertex_y[start_vertex_num];
						if (vertex_x[start_vertex_num]==vertex_x[end_vertex_num])
						{
							/*infinite gradient. Put in a large number! Anyway, we check for*/
							/* start_x=end_x when consume gradient[] in */
							/*get_triangle_scan_line_intersection, so never use */
							gradient[i]=1000000.0;
						}
						else
						{
							gradient[i]=delta_y/
								((float)vertex_x[end_vertex_num]-(float)vertex_x[start_vertex_num]);
						}
						if (delta_y==0.0)
						{
							/* zero length. Put in a large number for 1/length. This is never used*/
							inv_edge_y_length[i]=1000000.0;
						}
						else
						{
							inv_edge_y_length[i]=1.0/(delta_y);
						}
					}/* for(i=0;i<3;i++) */
					/*for each constant y "scan line"*/
					for(y=tri_min_y;y<=tri_max_y;y++)
					{
						number_of_intersected_edges=0;
						/*check and store which of the triangle's edges  are intersected*/
						/*by the constant y "scan line". There should be 2 or 3 edges */
						/*intersected. 2 edges, including the case of a single vertex,or 3 */
						/*if one of the edges is the "scan line", gradient==0 or if a vertex and */
						/*an edge are intersected. */
						if ((y>=vertex_y[0]&&y<=vertex_y[1])||(y>=vertex_y[1]&&y<=vertex_y[0]))
						{
							/*intersects 1st triangle edge, from vertex 0 to 1*/
							intersected_edges[number_of_intersected_edges]=0;
							number_of_intersected_edges++;
						}
						if ((y>=vertex_y[1]&&y<=vertex_y[2])||(y>=vertex_y[2]&&y<=vertex_y[1]))
						{
							/*intersects 2nd triangle edge from vertex 1 to 2*/
							intersected_edges[number_of_intersected_edges]=1;
							number_of_intersected_edges++;
						}
						if ((y>=vertex_y[2]&&y<=vertex_y[0])||(y>=vertex_y[0]&&y<=vertex_y[2]))
						{
							/*intersects 3rd triangle edgefrom  vertex 2 to 0 */
							intersected_edges[number_of_intersected_edges]=2;
							number_of_intersected_edges++;
						}
						/*find line across triangle*/
						if (number_of_intersected_edges==3)
						{
							/*If one of triangle edges has gradient==0,This is our line across triangle*/
							i=0;
							found=0;
							while((i<3)&&(!found))
							{
								if (gradient[i]==0)
								{
									found=1;
								}
								else
								{
									i++;
								}
							}
							if (found)
							{
								return_code=1;
								start_vertex_num=i;
								end_vertex_num=
									get_triangle_line_end_vertex_num_from_start_num(start_vertex_num);
								line_start_x=vertex_x[start_vertex_num];
								line_end_x=vertex_x[end_vertex_num];
								start_value=vertex_value[start_vertex_num];
								end_value=vertex_value[end_vertex_num];
							}/* if (found) */
							else
							{
								/*2 of the intersections will at a common vertex, the third in the middle*/
								/* of an edge (including the degenerate case, where this middle edge is a*/
								/* point, and the triangle is a line*) */
								for(i=0;i<3;i++)
								{
									triangle_egde_number=intersected_edges[i];
									start_vertex_num=triangle_egde_number;
									end_vertex_num=
										get_triangle_line_end_vertex_num_from_start_num(start_vertex_num);
									if (vertex_y[start_vertex_num]==y)
									{
										return_code=1;
										/*start intersection at a vertex */
										line_start_x=vertex_x[start_vertex_num];
										start_value=vertex_value[start_vertex_num];
										if (vertex_y[end_vertex_num]==y)
										{
											/*end intersection at a vertex */
											line_end_x=vertex_x[end_vertex_num];
											end_value=vertex_value[end_vertex_num];
										}
									}
									else if (vertex_y[end_vertex_num]!=y)
									{
										/*end intersection in the middle of an edge */
										return_code=get_triangle_scan_line_intersection(intersected_edges[i],
											y,vertex_x,vertex_y,gradient,inv_edge_y_length,vertex_value,
											&line_end_x,&end_value);
									}
								}/* for(i=0;i<3;i++)*/
							} /* if (found) */
						}
						else if (number_of_intersected_edges==2)
						{
							/*both of the intersections in the middle of an edge */
							return_code=(get_triangle_scan_line_intersection(intersected_edges[0],
								y,vertex_x,vertex_y,gradient,inv_edge_y_length,vertex_value,
								&line_start_x,&start_value)&&
								get_triangle_scan_line_intersection(intersected_edges[1],
									y,vertex_x,vertex_y,gradient,inv_edge_y_length,vertex_value,
									&line_end_x,&end_value));
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"draw_2d_construct_2d_gouraud_image_map. Wrong number of intersections");
							return_code=0;
						}
						if (return_code)
						{
							if (line_end_x==line_start_x)
							{
								delta_value=0;
							}
							else
							{
								delta_value=(end_value-start_value)/(line_end_x-line_start_x);
							}
							/*ensure line_start_x>line_end_x*/
							if (line_start_x>line_end_x)
							{
								x=line_start_x;
								line_start_x=line_end_x;
								line_end_x=x;
								value=start_value;
								start_value=end_value;
								end_value=value;
							}
							/*do line of pixels across the triangle*/
							value=start_value;
							for(x=line_start_x;x<=line_end_x;x++)
							{
#if defined (DEBUG)
								/*flag pixels at the triangle's edges*/
								if ((x==line_start_x)||(x==line_end_x)||(y==tri_min_y)||(y==tri_max_y))
								{
									edge_flag=1;
								}
								else
								{
									edge_flag=0;
								}
#endif /* defined (DEBUG)*/
								/*ensure the pixels of (wrapped) triangles at RH edge wrap to LH edge*/
								if (x>=(map_width-x_offset))
								{
									adjusted_x=x-map_width+2*x_offset;
								}
								else
								{
									adjusted_x=x;
								}
#if defined (DEBUG)
								/* draw pixels at triangle's edges dark, so can see easily*/
								if (edge_flag)
								{
									pixel_value[y*map_width+adjusted_x]=(*max_f);
								}
								else
#endif /* defined (DEBUG)*/
								{
									pixel_value[y*map_width+adjusted_x]=value;
								}
								background_map_boundary_base[y*map_width+adjusted_x]=1;
								value+=delta_value;
							}	/* for(x=line_start_x;x<=line_end_x;x++) */
						}/* if (return_code)*/
					}/* for(y=tri_min_y;y<=tri_max_y;y++) */
				}/* (k=0;k<map->number_of_gouraud_triangles;k++) */
			}/* if ((0!=stretch_x[region_number])&& */
		}/* if (	map_type==POTENTIAL)*/
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_construct_2d_gouraud_image_map. Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_construct_2d_gouraud_image_map*/

static int draw_2d_construct_image_map(struct Map *map,struct Sub_map *sub_map,
	struct Drawing_2d *drawing,int recalculate,int screen_region_height,
	int screen_region_width)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
Construct a colour map image for colour map or contours or  values  in the
<sub_map> frame.
*******************************************************************************/
{
	char *background_map_boundary,*background_map_boundary_base,*temp_char;
	Display *display;
	float background_pixel_value,boundary_pixel_value,max_f,min_f,*pixel_value;
	int bit_map_pad,bit_map_unit,contour_areas_in_x,contour_areas_in_y,
		*draw_region_number,i,map_width,map_height,maximum_x,maximum_y,minimum_x,
		minimum_y,number_of_contour_areas,number_of_regions,number_of_spectrum_colours,
		region_number,return_code,scan_line_bytes,temp_region_number;
	short int *contour_x,*contour_y;
	struct Map_drawing_information *drawing_information;
	struct Map_frame *frame;
	struct Region	*maximum_region,*minimum_region,*the_region;
	struct Region_list_item *region_item;
	struct Rig *rig;

	ENTER(draw_2d_construct_image_map);
	maximum_region=(struct Region *)NULL;
	minimum_region=(struct Region *)NULL;
	the_region=(struct Region *)NULL;
	region_item=(struct Region_list_item *)NULL;
	rig=(struct Rig *)NULL;
	display=(Display *)NULL;
	pixel_value=(float *)NULL;
	frame=(struct Map_frame *)NULL;
	drawing_information=(struct Map_drawing_information *)NULL;
	contour_x=(short int *)NULL;
	contour_y=(short int *)NULL;
	temp_char=(char *)NULL;
	background_map_boundary=(char *)NULL;
	background_map_boundary_base=(char *)NULL;
	if (map&&sub_map&&drawing&&(drawing_information=map->drawing_information))
	{
		return_code=1;
		map_width=sub_map->width;
		map_height=sub_map->height;
		draw_region_number=map->draw_region_number;
		rig= *(map->rig_pointer);
		number_of_regions=rig->number_of_regions;
		number_of_spectrum_colours=drawing_information->number_of_spectrum_colours;
		display=User_interface_get_display(drawing->user_interface);
		frame=&(sub_map->frame);
		if (recalculate||!(frame->pixel_values))
		{
			busy_cursor_on((Widget)NULL,
				drawing_information->user_interface);
			/* reallocate memory for frames */
			map->contour_x_spacing=
				drawing_information->pixels_between_contour_values;
			contour_areas_in_x=map_width/(map->contour_x_spacing);
			if (contour_areas_in_x<1)
			{
				contour_areas_in_x=1;
			}
			map->contour_x_spacing=map_width/contour_areas_in_x;
			if (map->contour_x_spacing*contour_areas_in_x<map_width)
			{
				if ((map->contour_x_spacing+1)*(contour_areas_in_x-1)<
					(map->contour_x_spacing-1)*(contour_areas_in_x+1))
				{
					map->contour_x_spacing++;
				}
				else
				{
					contour_areas_in_x++;
				}
			}
			map->contour_y_spacing=
				drawing_information->pixels_between_contour_values;
			contour_areas_in_y=map_height/(map->contour_y_spacing);
			if (contour_areas_in_y<1)
			{
				contour_areas_in_y=1;
			}
			map->contour_y_spacing=map_height/contour_areas_in_y;
			if ((map->contour_y_spacing)*contour_areas_in_y<map_height)
			{
				if ((map->contour_y_spacing+1)*(contour_areas_in_y-1)<
					(map->contour_y_spacing-1)*(contour_areas_in_y+1))
				{
					map->contour_y_spacing++;
				}
				else
				{
					contour_areas_in_y++;
				}
			}
			number_of_contour_areas=contour_areas_in_x*contour_areas_in_y;
			if ((recalculate>2)||!(frame->pixel_values))
			{
				/* the number of bits for each pixel */
				bit_map_unit=BitmapUnit(display);
				/* each scan line occupies a multiple of this number of
					bits */
				bit_map_pad=BitmapPad(display);
				scan_line_bytes=(((map_width*bit_map_unit-1)/
					bit_map_pad+1)*bit_map_pad-1)/8+1;
				/* allocate memory for drawing contour values */
				contour_x=(short int *)NULL;
				contour_y=(short int *)NULL;
				/*???DB.  This REALLOC causes unemap to crash when being
					run under XWin32 at Cedars Sinai */
				if (REALLOCATE(contour_x,frame->contour_x,short int,
					number_of_contour_areas*number_of_spectrum_colours)&&
					REALLOCATE(contour_y,frame->contour_y,short int,
						number_of_contour_areas*number_of_spectrum_colours))
				{
					frame->contour_x=contour_x;
					frame->contour_y=contour_y;
					/* allocate memory for pixel values */
					pixel_value=(float *)NULL;
					if (REALLOCATE(pixel_value,frame->pixel_values,float,
						map_width*map_height))
					{
						frame->pixel_values=pixel_value;
						/* allocate memory for image */
						if (frame->image)
						{
							DEALLOCATE(frame->image->data);
							XFree((char *)(frame->image));
							frame->image=(XImage *)NULL;
						}
						temp_char=(char *)NULL;
						if (ALLOCATE(temp_char,char,map_height*scan_line_bytes)&&
							(frame->image=XCreateImage(display,
								XDefaultVisual(display,XDefaultScreen(display)),
								drawing->depth,ZPixmap,0,temp_char,map_width,
								map_height,bit_map_pad,scan_line_bytes)))
						{
							/* initialize the contour areas */
							for (i=number_of_contour_areas*number_of_spectrum_colours;i>0;i--)
							{
								*contour_x= -1;
								*contour_y= -1;
								contour_x++;
								contour_y++;
							}
							sub_map->maximum_value=0;
							sub_map->minimum_value=0;
							frame->maximum_screen_x= -1;
							frame->maximum_screen_y= -1;
							frame->minimum_screen_x= -1;
							frame->minimum_screen_y= -1;
						}
						else
						{
							DEALLOCATE(temp_char);
							if (frame->image)
							{
								XFree((char *)(frame->image));
								frame->image=(XImage *)NULL;
							}
							DEALLOCATE(frame->contour_x);
							DEALLOCATE(frame->contour_y);
							DEALLOCATE(frame->pixel_values);
							display_message(ERROR_MESSAGE,
								"draw_2d_construct_image_map.  Insufficient memory for frame image");
							return_code=0;
						}
					}
					else
					{
						DEALLOCATE(frame->contour_x);
						DEALLOCATE(frame->contour_y);
						if (pixel_value)
						{
							/* this can't happen.  But included in case changes
								are made to allocation */
							DEALLOCATE(pixel_value);
							frame->pixel_values=(float *)NULL;
						}
						else
						{
							DEALLOCATE(frame->pixel_values);
						}
						display_message(ERROR_MESSAGE,
							"draw_2d_construct_image_map.  Insufficient memory for frame pixel values");
						return_code=0;
					}
				}
				else
				{
					if (contour_x)
					{
						DEALLOCATE(contour_x);
						frame->contour_x=(short int *)NULL;
						if (contour_y)
						{
							/* this can't happen.  But included in case changes
								are made to allocation */
							DEALLOCATE(contour_x);
							frame->contour_x=(short int *)NULL;
						}
						else
						{
							DEALLOCATE(frame->contour_y);
						}
					}
					else
					{
						DEALLOCATE(frame->contour_x);
						DEALLOCATE(frame->contour_y);
					}
					display_message(ERROR_MESSAGE,
						"draw_2d_construct_image_map.  Insufficient memory for frame contour values");
					return_code=0;
				}
				if (return_code)
				{
					map->number_of_contour_areas=number_of_contour_areas;
					map->number_of_contour_areas_in_x=contour_areas_in_x;
				}
				else
				{
					map->number_of_contour_areas=0;
					map->number_of_contour_areas_in_x=0;
					/* free memory */
					DEALLOCATE(frame->image->data);
					XFree((char *)(frame->image));
					frame->image=(XImage *)NULL;
					DEALLOCATE(frame->contour_x);
					DEALLOCATE(frame->contour_y);
					DEALLOCATE(frame->pixel_values);
				}
			}
			if (return_code)
			{
				if (recalculate>1)
				{
					/* allocate memory for drawing the map boundary */
					if (ALLOCATE(background_map_boundary_base,char,map_width*map_height))
					{
						/*???DB.  Put the frame loop on the inside ? */
						frame=&(sub_map->frame);
						pixel_value=frame->pixel_values;
						/* clear the image */
						background_map_boundary=background_map_boundary_base;
						for (i=map_width*map_height;i>0;i--)
						{
							*background_map_boundary=0;
							background_map_boundary++;
						}
						min_f=1;
						max_f=0;
						maximum_region=(struct Region *)NULL;
						minimum_region=(struct Region *)NULL;
						/* for each region */
						region_item=get_Rig_region_list(rig);
						for (temp_region_number=0;
								temp_region_number<number_of_regions;
								temp_region_number++)
						{
							region_number=
								draw_region_number[temp_region_number];
							if (0<=region_number)
							{
								the_region=get_Region_list_item_region(
									region_item);
								if ((map->projection_type==CYLINDRICAL_PROJECTION)&&
									(map->interpolation_type==DIRECT_INTERPOLATION)&&
									(*(map->type)==POTENTIAL)&&
									the_region&&(the_region->type==TORSO))
								{
									draw_2d_construct_2d_gouraud_image_map(map,sub_map,region_number,
										the_region,&maximum_region,&minimum_region,
										&minimum_x,&minimum_y,&maximum_x,&maximum_y,&max_f,&min_f,
										background_map_boundary_base);
								}
								else
								{
									draw_2d_construct_interpolated_image_map(map,sub_map,region_number,
										the_region,&maximum_region,&minimum_region,
										screen_region_height,screen_region_width,&minimum_x,&minimum_y,
										&maximum_x,&maximum_y,&max_f,&min_f,background_map_boundary_base);
								}
							}/* if (0<=region_number) */
							region_item=get_Region_list_item_next(region_item);
						}/* for (temp_region_number=0; */
						frame->maximum_region=maximum_region;
						frame->minimum_region=minimum_region;
						set_sub_map_min_max(sub_map,max_f,min_f,minimum_x,
							maximum_x,minimum_y,maximum_y);
						if (max_f<min_f)
						{
							background_pixel_value= -1;
							boundary_pixel_value=1;
						}
						else
						{
							background_pixel_value=min_f-1;
							boundary_pixel_value=max_f+1;
							if ((background_pixel_value>=min_f)||
								(boundary_pixel_value<=max_f))
							{
								display_message(ERROR_MESSAGE,
									"draw_2d_construct_image_map.  Problems with background/boundary");
							}
						}/* if (max_f<min_f) */
						draw_2d_map_boundary(background_map_boundary_base,
							map_height,map_width,pixel_value,boundary_pixel_value,
							background_pixel_value);
						DEALLOCATE(background_map_boundary_base);
					}/* if (ALLOCATE(background_map_boundary_base,char, */
					else
					{
						display_message(ERROR_MESSAGE,"draw_2d_construct_image_map.  ",
							"Insufficient memory for background_map_boundary_base");
					}
				}/*	if (recalculate>1) */
			} /* if (return_code) */
			busy_cursor_off((Widget)NULL,drawing_information->user_interface);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_construct_image_map Invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_construct_image_map*/

static int draw_2d_make_map(struct Map *map,int recalculate,
	struct Drawing_2d *drawing,int map_width,int map_height,int map_x_offset,
	int map_y_offset,float frame_time,int use_potential_time)
/*******************************************************************************
LAST MODIFIED : 19 November 2001

DESCRIPTION :
This function draws the <map> in the <drawing>, with <map_width>, <map_height>
at <map_x_offset> <map_y_offset>. <map_width>, <map_height> <map_x_offset>
<map_y_offset> must be inside drawing->width,drawing->height. (This is checked)
If <recalculate> is >0 then the colours for the pixels are recalculated.
If <recalculate> is >1 then the interpolation functions are also recalculated.
If <recalculate> is >2 then the <map> is resized to match the <drawing>.

???Would like to develop a "PostScript driver" for X.  To get an idea of whats
involved I'll put a PostScript switch into this routine so that it either draws
to the drawing or writes to a postscript file.
???DB.  Use XDrawSegments for contours ?
==============================================================================*/
{
	char *electrode_drawn=(char *)NULL;
	char *first=(char *)NULL;
	Display *display=(Display *)NULL;
	enum Map_type map_type;
	float a,f_value;
	float *electrode_value=(float *)NULL;
	float *min_x,*min_y=(float *)NULL;
	float *stretch_y=(float *)NULL;
	float *stretch_x=(float *)NULL;
	float *x=(float *)NULL;
	float *x_item=(float *)NULL;
	float *y=(float *)NULL;
	float *y_item=(float *)NULL;
	int ascent,descent,direction,*draw_region_number,i,number_of_devices,
		number_of_drawn_regions,number_of_electrodes,number_of_regions,
		region_number,return_code,screen_region_height,screen_region_width,*times,
		x_border,x_name_border,y_border,y_name_border;
	int *screen_x=(int *)NULL;
	int *screen_y=(int *)NULL;
	int *start_x=(int *)NULL;
	int *start_y=(int *)NULL;
	struct Device **device=(struct Device **)NULL;
	struct Device	**electrode=(struct Device **)NULL;
	struct Device_description *description=
		(struct Device_description *)NULL;
	struct Map_drawing_information *drawing_information=
		(struct Map_drawing_information *)NULL;
	struct Region *current_region=(struct Region *)NULL;
	struct Rig *rig=(struct Rig *)NULL;
	struct Sub_map *sub_map=(struct Sub_map *)NULL;
	struct Signal_buffer *buffer;
	XCharStruct bounds;
	XFontStruct *font=(XFontStruct *)NULL;

	ENTER(draw_2d_make_map);
	return_code=1;
	/* check arguments */
	if (map&&drawing&&(drawing_information=map->drawing_information)&&
		(drawing_information->user_interface)&&
		(drawing_information->user_interface=drawing->user_interface)&&
		(map_width+map_x_offset<=drawing->width)&&
		(map_height+map_y_offset<=drawing->height))
		/*???DB.  Am I going overboard with getting rid of globals ?  Should display
			be a global ? */
	{
		display=User_interface_get_display(drawing->user_interface);	
		font=drawing_information->font;
		if (map->type)
		{
			map_type= *(map->type);
		}
		else
		{
			map_type=NO_MAP_FIELD;
		}
		if (map_type==NO_MAP_FIELD)
			/* if we have no map, don't colour electrodes with the signal  */
		{
			map->colour_electrodes_with_signal=0;
		}
		switch (map_type)
		{
			case SINGLE_ACTIVATION:
			{
				if (!map->event_number)
				{
					display_message(ERROR_MESSAGE,"draw_2d_make_map.  Missing event_number");
				}
				if (!map->datum)
				{
					display_message(ERROR_MESSAGE,"draw_2d_make_map.  Missing datum");
				}
			} break;
			case MULTIPLE_ACTIVATION:
			{
				if (!map->datum)
				{
					display_message(ERROR_MESSAGE,"draw_2d_make_map.  Missing datum");
				}
			} break;
			case INTEGRAL:
			{
				if (!map->start_search_interval)
				{
					display_message(ERROR_MESSAGE,
						"draw_2d_make_map.  Missing start_search_interval");
				}
				if (!map->end_search_interval)
				{
					display_message(ERROR_MESSAGE,
						"draw_2d_make_map.  Missing end_search_interval");
				}

			} break;
			case POTENTIAL:
			{
				;
			} break;
		}/* switch (map_type) */
		if (map->rig_pointer)
		{
			if ((rig= *(map->rig_pointer))&&
				(0<(number_of_regions=rig->number_of_regions)))
			{
				DEALLOCATE(map->draw_region_number);
				if (ALLOCATE(draw_region_number,int,number_of_regions))
				{
					for (i=0;i<number_of_regions;i++)
					{
						draw_region_number[i]=0;
					}
					current_region=get_Rig_current_region(rig);
					/* determine the number of regions to draw and count the electrodes */
					/*include rejected electrodes, as want to draw these under some circumstances*/
					number_of_electrodes=0;
					device=rig->devices;
					number_of_devices=rig->number_of_devices;
					while (number_of_devices>0)
					{
						if ((ELECTRODE==(description=(*device)->description)->type)&&
							(!current_region||(current_region==description->region)))
						{
							draw_region_number[description->region->number]++;
							number_of_electrodes++;
						}
						device++;
						number_of_devices--;
					}
					number_of_drawn_regions=0;
					for (i=0;i<number_of_regions;i++)
					{
						if (0<draw_region_number[i])
						{
							draw_region_number[i]=number_of_drawn_regions;
							number_of_drawn_regions++;
						}
						else
						{
							draw_region_number[i]= -1;
						}
					}/* for (i=0;i<number_of_regions;i++) */
					map->number_of_electrodes=number_of_electrodes;
					map->number_of_drawn_regions=number_of_drawn_regions;
					map->draw_region_number=draw_region_number;
					DEALLOCATE(map->electrodes);
					DEALLOCATE(map->electrode_drawn);
					if (map->number_of_electrodes)
					{
						if (ALLOCATE(electrode,struct Device *,number_of_electrodes)&&
							ALLOCATE(electrode_drawn,char,number_of_electrodes)&&
							ALLOCATE(x,float,number_of_electrodes)&&
							ALLOCATE(y,float,number_of_electrodes)&&
							ALLOCATE(first,char,number_of_drawn_regions))
						{
							map->electrodes=electrode;
							map->electrode_drawn=electrode_drawn;
							if ((sub_map=create_Sub_map(number_of_drawn_regions,
								number_of_electrodes,map_x_offset,map_y_offset,map_height,
								map_width,frame_time,use_potential_time))&&
								(map_add_Sub_map(map,sub_map)))
							{
								if (use_potential_time)
								{
									if ((map->potential_time)&&(rig->devices)&&(*(rig->devices))&&
										((*(rig->devices))->signal)&&
										(buffer=(*(rig->devices))->signal->buffer)&&
										(times=buffer->times))
									{
										sub_map->frame_time=(float)((times)[*(map->potential_time)])*
											1000./(buffer->frequency);
									}
									else
									{
										sub_map->frame_time=1;
									}
								}
								screen_x=sub_map->electrode_x;
								screen_y=sub_map->electrode_y;
								electrode_value=sub_map->electrode_value;
								min_x=sub_map->min_x;
								min_y=sub_map->min_y;
								stretch_x=sub_map->stretch_x;
								stretch_y=sub_map->stretch_y;
								start_x=sub_map->start_x;
								start_y=sub_map->start_y;
								/* calculate the projections of the electrode positions, the
									electrode values and the x and y ranges */
								device=rig->devices;
								number_of_devices=rig->number_of_devices;
								x_item=x;
								y_item=y;
								for (i=number_of_drawn_regions;i>0;)
								{
									i--;
									first[i]=1;
								}
								x_border=4;
								y_border=4;
								while (number_of_devices>0)
								{
									if ((ELECTRODE==(description=(*device)->description)->type)&&
										(!current_region||(current_region==description->region)))
									{
										/* add device */
										*electrode= *device;
										/* update border size */
										if (description->name)
										{
											XTextExtents(font,description->name,
												strlen(description->name),&direction,&ascent,&descent,
												&bounds);
											x_name_border=bounds.lbearing+bounds.rbearing+1;
											if (x_name_border>x_border)
											{
												x_border=x_name_border;
											}
											y_name_border=ascent+1+descent;
											if (y_name_border>y_border)
											{
												y_border=y_name_border;
											}
										}
										/* perform projection and update ranges */
										draw_2d_perform_projection_update_ranges(map,sub_map,
											description,x_item,y_item,first);
										/* calculate electrode value */
										f_value=0;
										*electrode_drawn=0;
										draw_2d_calculate_electrode_value(map,sub_map,electrode_drawn,
											electrode,&f_value);
										*electrode_value=f_value;
										electrode_value++;
										electrode_drawn++;
										electrode++;
										x_item++;
										y_item++;
									}/* if ((ELECTRODE==(description=(*device)->description)->type)&& */
									device++;
									number_of_devices--;
								}/* while (number_of_devices>0) */
								draw_2d_set_min_x_max_x(map,sub_map,&a,map->projection_type);
								map->number_of_region_columns=(int)(0.5+sqrt((double)number_of_drawn_regions));
								if (map->number_of_region_columns<1)
								{
									map->number_of_region_columns=1;
								}
								map->number_of_region_rows=(number_of_drawn_regions-1)/
									map->number_of_region_columns+1;
								if (map->number_of_region_rows<1)
								{
									map->number_of_region_rows=1;
								}
								/* equalize the columns */
								if ((number_of_drawn_regions>1)&&(map->number_of_region_columns>1)&&
									((i=map->number_of_region_rows*map->number_of_region_columns-
										number_of_drawn_regions-1)>0))
								{
									map->number_of_region_rows -= i/(map->number_of_region_columns-1);
								}
								/* make the regions fill the width and the height */
								screen_region_width=(map_width)/map->number_of_region_columns;
								screen_region_height=(map_height)/map->number_of_region_rows;
								draw_2d_calc_map_to_screen_transform(map,sub_map,display,
									screen_region_width,screen_region_height,x_border,y_border,ascent,
									descent);
								/* calculate the electrode screen locations */
								electrode=map->electrodes;
								x_item=x;
								y_item=y;
								screen_x=sub_map->electrode_x;
								screen_y=sub_map->electrode_y;
								for (i=number_of_electrodes;i>0;i--)
								{
									region_number=draw_region_number[
										(*electrode)->description->region->number];
									/* calculate screen position */
									*screen_x=start_x[region_number]+
										(int)(((*x_item)-min_x[region_number])*
											stretch_x[region_number]) +map_x_offset;
									*screen_y=start_y[region_number]-
										(int)(((*y_item)-min_y[region_number])*
											stretch_y[region_number]) +map_y_offset;
									electrode++;
									x_item++;
									y_item++;
									screen_x++;
									screen_y++;
								}/* for (i=number_of_electrodes;i>0;i--) */
								/* construct a colour map image for colour map or contours or
									values */
								/* draw colour map and contours first (background) */
								if (NO_MAP_FIELD!=map_type)
								{
									if (NO_INTERPOLATION!=map->interpolation_type)
									{
										draw_2d_construct_image_map(map,sub_map,drawing,recalculate,
											screen_region_height,screen_region_width);
									}/* (NO_INTERPOLATION!=map->interpolation_type) */
									else
									{
										if (1<recalculate)
										{
											set_map_2d_no_interpolation_min_max(map,sub_map);
										}
									}
								}/* if (NO_MAP_FIELD!=map_type) */
							}/* if (x&&y&&electrode&&screen_x&&screen_y&&electrode_value&& */
							else
							{
								display_message(ERROR_MESSAGE,
									"draw_2d_make_map. creation of sub_map failed");
								return_code=0;
							}
							DEALLOCATE(x);
							DEALLOCATE(y);
							DEALLOCATE(first);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"draw_2d_make_map. out of memory for electrode/electrode_drawnx/y/first");
							return_code=0;
						}
					}
					else
					{
						map->minimum_value=0;
						map->maximum_value=1;
						map->contour_minimum=0;
						map->contour_maximum=1;
						display_message(WARNING_MESSAGE,"No electrodes to map!");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"draw_2d_make_map. out of memory for draw_region_number");
					return_code=0;
				}
			} /* if ((rig= *(map->rig_pointer))&& */
		}/* if (map->rig_pointer) */
	}/* if (map&&drawing&&(drawing_information=map->drawing_information)&& */
	else
	{
		display_message(ERROR_MESSAGE,"draw_2d_make_map.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* draw_2d_make_map */

static int draw_2d_electrical_imaging_maps(struct Map *map,int recalculate,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 17 October 2001

DESCRIPTION:
Draw multiple sub maps on the same window for Electrical Imaging.
==============================================================================*/
{
	float frame_time;
	int num_maps,sub_map_rows,sub_map_cols,count,x_step,y_step,j,map_width,
		map_height,map_x_offset,map_y_offset,return_code,*times;
	struct Electrical_imaging_event *eimaging_event;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	struct Rig *rig;
	struct Device *device;

	ENTER(draw_2d_electrical_imaging_maps);
	eimaging_event=(struct Electrical_imaging_event *)NULL;
	signal=(struct Signal *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	times=(int *)NULL;
	rig=(struct Rig *)NULL;
	device=(struct Device *)NULL;
	if (map&&drawing)
	{
		return_code=1;
		rig= *(map->rig_pointer);
		device=*(rig->devices);
		signal=(device)->signal;
		buffer=signal->buffer;
		times=buffer->times;
		eimaging_event=*map->first_eimaging_event;
		frame_time=(int)((float)((times)[*(map->potential_time)])
			*1000./buffer->frequency);
		/* will have num_maps sub_maps, but not displaying as movie, so set*/
		/* map->number_of_frames to 1. Not used in this function, anyway */
		map->number_of_frames=1;
		map->start_time=frame_time;
		map->end_time=map->start_time;
		if (recalculate>0)
		{
			/* Destroy any existing sub maps */
			map_destroy_Sub_maps(map);
			/*get size and position of sub_maps*/
			get_number_of_sub_maps_x_step_y_step_rows_cols(map,drawing,&num_maps,
				&x_step,&y_step,&sub_map_rows,&sub_map_cols);
			map_width=drawing->width/sub_map_rows;
			map_height=drawing->height/sub_map_cols;
			map_x_offset=0;
			map_y_offset=0;
			count=0;
			j=1;
			/* loop through and create all the sub maps*/
			while (count<num_maps)
			{
				/* set the sub_map's times from the electrical imaging events */
				frame_time=(int)((float)((times)[eimaging_event->time])
					*1000./buffer->frequency+0.5);
				return_code=draw_2d_make_map(map,recalculate,drawing,map_width,map_height,
					map_x_offset,map_y_offset,frame_time,0);
				map_x_offset+=x_step;
				count++;
				j++;
				if (j>sub_map_rows)
				{
					j=1;
					map_x_offset=0;
					map_y_offset+=y_step;
				}
				eimaging_event=eimaging_event->next;
			}
			if (recalculate>1)
			{
				set_map_2d_map_min_max(map);
			}
		}/* if (recalculate>0) */
		update_colour_map_unemap(map,drawing);
		/* loop through and draw all the sub maps*/
		count=0;
		while(count<map->number_of_sub_maps)
		{
			draw_2d_show_map(map,count,drawing);
			count++;
		}
		map->sub_map_number=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_2d_electrical_imaging_maps. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* draw_2d_electrical_imaging_maps */

static int draw_map_2d(struct Map *map,int recalculate,
	struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 30 October 2001

DESCRIPTION:
Draws all the sub maps of the 2D <map>, by first making it with
draw_2d_make_map, then drawing it with draw_2d_show_map. Arranges the sub maps
on the page.
See draw_2d_make_map for meanings of recalculate.
==============================================================================*/
{
	float end_time,start_time,frame_time;
	int frame_number,number_of_frames,map_width,map_height,map_x_offset,
		map_y_offset,return_code,use_potential_time;

	ENTER(draw_map_2d);
	return_code=0;
	if (map&&drawing)
	{
		return_code=1;
		if ((*map->first_eimaging_event)&&
			(ELECTRICAL_IMAGING==*map->analysis_mode)&&(*(map->type)==POTENTIAL))
		{
			/* draw lots of little maps*/
			draw_2d_electrical_imaging_maps(map,recalculate,drawing);
		}
		else
		{
			if ((recalculate>0)&&(recalculate<3))
			{
				/*we've stopped making movies (3=making) (0=displaying frames) */
				map->number_of_frames=1;
			}
			if (recalculate==0)
			{
				/* just draw the current frame */
				draw_2d_show_map(map,map->sub_map_number,drawing);
			}
			else
			{
				/*making frame(s) */
				/* Destroy any existing sub maps */
				map_destroy_Sub_maps(map);
				map_width=drawing->width;
				map_height=drawing->height;
				map_x_offset=0;
				map_y_offset=0;
				map->sub_map_number=0;
				start_time=map->start_time;
				end_time=map->end_time;
				number_of_frames=map->number_of_frames;
				frame_number=0;
				/* make the sub_maps(frames) */
				for(frame_number=0;frame_number<number_of_frames;frame_number++)
				{
					if (number_of_frames==1)
					{
						use_potential_time=1;
						frame_time=0;
					}
					else
					{
						use_potential_time=0;
						frame_time=((float)(number_of_frames-frame_number-1)*(start_time)+
							(float)frame_number*(end_time))/(float)(number_of_frames-1);
					}
					return_code=draw_2d_make_map(map,recalculate,drawing,map_width,
						map_height,map_x_offset,map_y_offset,frame_time,use_potential_time);
				}
				if (return_code&&map->number_of_electrodes)
				/* map->number_of_electrodes=0, nothing to show*/
				{
					if (number_of_frames>1)
					{
						/* draw the first frame of movie */
						set_map_2d_map_min_max(map);
					}
					else
					{
						if (recalculate>1)
						{
							set_map_2d_map_min_max(map);
						}
					}
					update_colour_map_unemap(map,drawing);
					draw_2d_show_map(map,map->sub_map_number,drawing);
				} /* if (return_code&&map->number_of_electrodes)	*/
			}/* if (recalculate==0) */
		}/* if ((*map->first_eimaging_event)&& */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_map_2d. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_map_2d */

enum Projection_type ensure_map_projection_type_matches_region_type(
	struct Map *map)
/*******************************************************************************
LAST MODIFIED : 20 September 2001

DESCRIPTION : Ensure that the map->projection_type and the
rig->current_region->type are compatible.
==============================================================================*/
{
	enum Projection_type projection_type;
	struct Rig *rig;

	ENTER(ensure_projection_type_matches_region_type);
	/* the default */
	projection_type=HAMMER_PROJECTION;
	if (map)
	{
		if (rig=*(map->rig_pointer))
		{
			projection_type=map->projection_type;
#if defined (UNEMAP_USE_3D)
			/*THREED_PROJECTION always valid */
			if (projection_type!=THREED_PROJECTION)
			{
#endif /* defined (UNEMAP_USE_3D) */
				/*possibly change the projection_type*/
				if (!rig->current_region)
				{
					/*no current_region, so mutli region */
					projection_type=HAMMER_PROJECTION;
				}
				else
				{
					switch (rig->current_region->type)
					{
						case SOCK:
						{
							if (projection_type==CYLINDRICAL_PROJECTION)
							{
								projection_type=HAMMER_PROJECTION;
							}
						} break;
						case PATCH:
						{
							if ((projection_type==HAMMER_PROJECTION)||
								(projection_type==POLAR_PROJECTION))
							{
								projection_type=CYLINDRICAL_PROJECTION;
							}
						} break;
						case TORSO:
						{
							if ((projection_type==HAMMER_PROJECTION)||
								(projection_type==POLAR_PROJECTION))
							{
								projection_type=CYLINDRICAL_PROJECTION;
							}
						} break;
						default:
						{
							projection_type=HAMMER_PROJECTION;
						} break;
					}
				}
#if defined (UNEMAP_USE_3D)
			}
#endif /* defined (UNEMAP_USE_3D) */
			/*set the projection type*/
			map->projection_type=projection_type;
		} /* if (rig=*(map->rig_pointer)) */
	}/* if (map) */
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_map_projection_type_matches_region_type. Invalid argument");
	}
	LEAVE;
	
	return (projection_type);
}/* ensure_projection_type_matches_region_type*/

static enum Interpolation_type ensure_interpolation_type_matches_region_type(
	struct Map *map)
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION:
Ensures the map->interpolation_type and rig->current_region match.
Also calls ensure_map_projection_type_matches_region_type if they don't
==============================================================================*/
{
	struct Rig *rig;
	enum Interpolation_type type;

	ENTER(ensure_interpolation_type_matches_region_type);
	/*make no interpolation the default*/
	type=NO_INTERPOLATION;
	if (map)
	{
		rig=(struct Rig *)NULL;
		/*only currently a problem for DIRECT_INTERPOLATION*/
		if (map->interpolation_type==DIRECT_INTERPOLATION)
		{
			rig=*(map->rig_pointer);
			if (!((rig->current_region)&&(rig->current_region->type==TORSO)))
			{
				map->interpolation_type=BICUBIC_INTERPOLATION;
				ensure_map_projection_type_matches_region_type(map);
			}
		}/* if (map->interpolation_type==DIRECT_INTERPOLATION) */
		type=map->interpolation_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_interpolation_type_matches_region_type. Invalid argument");
	}
	LEAVE;
	
	return (type);
}/* ensure_interpolation_type_matches_region_type */

int draw_map(struct Map *map,int recalculate,struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 17 October 2001

DESCRIPTION:
Call draw_map_2d or draw_map_3d depending upon <map>->projection_type.
==============================================================================*/
{
	int return_code;

	ENTER(draw_map);
	if (map)
	{
		if ((map->rig_pointer)&&(*(map->rig_pointer)))
		{
			ensure_interpolation_type_matches_region_type(map);
#if defined (UNEMAP_USE_3D)
			/* 3d map for 3d projection */
			if (map->projection_type==THREED_PROJECTION)
			{
				return_code=draw_map_3d(map);
				/*???JW.  update_colour_map_unemap() necessary for old style colour
					strip at top of mapping window. Will become obselete when only cmgui
					methods used  */
				update_colour_map_unemap(map,drawing);
			}
			else
			{
				draw_map_2d(map,recalculate,drawing);
			}
#else /* defined (UNEMAP_USE_3D) */
			/* 2d map */
			draw_map_2d(map,recalculate,drawing);
#endif /* defined( UNEMAP_USE_3D) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_map invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_map */

int draw_colour_or_auxiliary_area(struct Map *map,struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 21 June 1997
DESCRIPTION :
This function draws the colour bar or the auxiliary inputs in the <drawing>.
It should not be called until draw_map has been called.
==============================================================================*/
{
	char value_string[11];
	Display *display;
	float contour_maximum,contour_minimum,contour_value,contour_x,maximum_value,
		minimum_value,spectrum_left,spectrum_right;
	GC graphics_context;
	int ascent,colour_bar_bottom,colour_bar_left,colour_bar_right,colour_bar_top,
		descent,direction,first,i,name_end,number_of_auxiliary,number_of_devices,
		number_of_spectrum_colours,return_code,*screen_x,*screen_y,string_length,
#if defined (NO_ALIGNMENT)
		text_x,text_y,
#endif /* defined (NO_ALIGNMENT) */
		widget_spacing,x,xmarker,x_range,yheight,ymarker;
	Pixel *spectrum_pixels;
	struct Device_description *description;
	struct Device **auxiliary,**device;
	struct Map_drawing_information *drawing_information;
	struct Region *current_region;
	struct Rig *rig;
	XCharStruct bounds;
	XFontStruct *font;

	ENTER(draw_colour_or_auxiliary_area);
	return_code=1;
	/* check arguments */
	if (map&&drawing&&(drawing_information=map->drawing_information)&&
		(drawing_information->user_interface)&&
		(drawing_information->user_interface=drawing->user_interface))
	{
		display=User_interface_get_display(drawing->user_interface);
		widget_spacing=User_interface_get_widget_spacing(drawing->user_interface);
		spectrum_pixels=drawing_information->spectrum_colours;
		number_of_spectrum_colours=drawing_information->number_of_spectrum_colours;
		font=drawing_information->font;
		if ((SHOW_COLOUR==map->colour_option)||
			(SHOW_CONTOURS==map->contours_option))
		{
			/* clear the auxiliary positions */
			if (map->number_of_auxiliary)
			{
				map->number_of_auxiliary=0;
				DEALLOCATE(map->auxiliary);
				DEALLOCATE(map->auxiliary_x);
				DEALLOCATE(map->auxiliary_y);
			}
			/* draw the colour bar */
			/*???Use XImage ? */
			if (map->rig_pointer)
			{
				if (rig= *(map->rig_pointer))
				{
					minimum_value=map->minimum_value;
					maximum_value=map->maximum_value;
					contour_minimum= map->contour_minimum;
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
						(drawing_information->graphics_context).spectrum_text_colour,
#if defined (NO_ALIGNMENT)
						(int)(text_x+0.5),text_y,value_string,string_length);
#else
					(int)(spectrum_left+0.5),widget_spacing,value_string,string_length);
#endif
				/* write the maximum value */
				sprintf(value_string,"%.4g",contour_maximum);
				string_length=strlen(value_string);
				XTextExtents(font,value_string,string_length,&direction,&ascent,
					&descent,&bounds);
#if defined (NO_ALIGNMENT)
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
					(drawing_information->graphics_context).spectrum_text_colour,
#if defined (NO_ALIGNMENT)
					(int)(text_x+0.5),text_y,value_string,string_length);
#else
				(int)(spectrum_right+0.5),widget_spacing,value_string,
					string_length);
#endif
			colour_bar_top=ascent+descent+3*widget_spacing;
			colour_bar_bottom=drawing->height-widget_spacing;
			/* draw the colour bar */
			graphics_context=(drawing_information->graphics_context).spectrum;
			if ((colour_bar_left<colour_bar_right)&&
				(colour_bar_top<colour_bar_bottom))
			{
				x_range=colour_bar_right-colour_bar_left;
				for (x=colour_bar_left;x<=colour_bar_right;x++)
				{
					XSetForeground(display,graphics_context,
						spectrum_pixels[(int)((float)((x-colour_bar_left)*
							(number_of_spectrum_colours-1))/(float)x_range+0.5)]);
					XPSFillRectangle(display,drawing->pixel_map,graphics_context,
						x,colour_bar_top,1,colour_bar_bottom-colour_bar_top);
				}
			}
			if ((SHOW_CONTOURS==map->contours_option)&&
				(2<map->number_of_contours)&&(contour_minimum<contour_maximum))
			{
				/* draw the contour markers */
#if !defined (NO_ALIGNMENT)
				SET_HORIZONTAL_ALIGNMENT(CENTRE_HORIZONTAL_ALIGNMENT);
#endif
				graphics_context=(drawing_information->graphics_context).
					contour_colour;
				XPSDrawLineFloat(display,drawing->pixel_map,graphics_context,
					spectrum_left,(float)(colour_bar_top),spectrum_left,
					(float)colour_bar_bottom);
				for (i=(map->number_of_contours)-2;i>0;i--)
				{
					contour_x=(spectrum_right*(float)i+spectrum_left*
						(float)(map->number_of_contours-i-1))/
						(float)(map->number_of_contours-1);
					XPSDrawLineFloat(display,drawing->pixel_map,graphics_context,
						contour_x,(float)colour_bar_top,contour_x,
						(float)colour_bar_bottom);
					/* write the contour value */
					contour_value=(contour_maximum*(float)i+contour_minimum*
						(float)(map->number_of_contours-i-1))/
						(float)(map->number_of_contours-1);
					if (fabs(contour_value)<
						0.00001*(fabs(contour_maximum)+fabs(contour_minimum)))
					{
						contour_value=0;
					}
					sprintf(value_string,"%.4g",contour_value);
					string_length=strlen(value_string);
#if defined (NO_ALIGNMENT)
					XTextExtents(font,value_string,string_length,&direction,&ascent,
						&descent,&bounds);
					text_x=contour_x+(float)(bounds.lbearing-bounds.rbearing)/2;
					text_y=widget_spacing+ascent;
#endif
						XPSDrawString(display,drawing->pixel_map,graphics_context,
#if defined (NO_ALIGNMENT)
							(int)(text_x+0.5),text_y,value_string,string_length);
#else
						(int)(contour_x+0.5),widget_spacing,value_string,
							string_length);
#endif
				}
				XPSDrawLineFloat(display,drawing->pixel_map,graphics_context,
					spectrum_right,(float)(colour_bar_top),spectrum_right,
					(float)colour_bar_bottom);
			}
			/* draw the spectrum left and right markers */
			graphics_context=(drawing_information->graphics_context).
				spectrum_text_colour;
			XPSDrawLineFloat(display,drawing->pixel_map,graphics_context,
				spectrum_left,(float)(colour_bar_top-widget_spacing),spectrum_left,
				(float)colour_bar_top);
			XPSDrawLineFloat(display,drawing->pixel_map,graphics_context,
				spectrum_right,(float)(colour_bar_top-widget_spacing),
				spectrum_right,(float)colour_bar_top);
			/* save values */
			map->colour_bar_left=colour_bar_left;
			map->colour_bar_right=colour_bar_right;
			map->colour_bar_bottom=colour_bar_bottom;
			map->colour_bar_top=colour_bar_top;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
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
						if ((AUXILIARY==(description=(*device)->description)->type)&&
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
						if (ALLOCATE(map->auxiliary,struct Device *,number_of_auxiliary)&&
							ALLOCATE(map->auxiliary_x,int,number_of_auxiliary)&&
							ALLOCATE(map->auxiliary_y,int,number_of_auxiliary))
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
								if ((AUXILIARY==(description=(*device)->description)->type)&&
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
												(drawing_information->graphics_context).
												highlighted_colour,xmarker-2,ymarker-2,5,5);
											if (description->name)
											{
												XPSDrawString(display,drawing->pixel_map,
													(drawing_information->graphics_context).
													highlighted_colour,xmarker+6-bounds.lbearing,
													ymarker+ascent-4,description->name,string_length);
											}
										}
										else
										{
											XPSFillRectangle(display,drawing->pixel_map,
												(drawing_information->graphics_context).
												unhighlighted_colour,xmarker-2,ymarker-2,5,5);
											if (description->name)
											{
												XPSDrawString(display,drawing->pixel_map,
													(drawing_information->graphics_context).
													node_text_colour,xmarker+6-bounds.lbearing,
													ymarker+ascent-4,description->name,string_length);
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
							display_message(ERROR_MESSAGE,
		"draw_colour_or_auxiliary_area.  Could not allocate auxiliary information");
							return_code=0;
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"draw_colour_or_auxiliary_area.  NULL map->rig_pointer");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_colour_or_auxiliary_area.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_colour_or_auxiliary_area */

#if defined (TEST_TRUE_COLOUR_VISUAL)
/*???debug.  To test true colour visuals */
extern Colormap default_colour_map;
extern Visual *default_visual;
#endif /* defined (TEST_TRUE_COLOUR_VISUAL) */

struct Map_drawing_information *create_Map_drawing_information(
	struct User_interface *user_interface
#if defined (UNEMAP_USE_3D)
	,struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Light_model) *light_model_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(FE_node) *data_manager,
	struct LIST(GT_object) *glyph_list,
	struct Graphical_material *graphical_material,
	struct Computed_field_package *computed_field_package,
	struct Light *light,
	struct Light_model *light_model
#endif /* defined (UNEMAP_USE_3D) */
	)
/*******************************************************************************
LAST MODIFIED : 3 May 2001

DESCRIPTION :
==============================================================================*/
{
	Colormap colour_map;
	Display *display;
	int depth,i,number_of_visuals,number_of_spectrum_colours,screen_number;
	Pixmap depth_screen_drawable;
	Pixel *spectrum_colours;
	struct Map_drawing_information *map_drawing_information;
#define XmNcolourBarTextColour "colourBarTextColour"
#define XmCColourBarTextColour "ColourBarTextColour"
#define XmNcontourColour "contourColour"
#define XmCContourColour "ContourColour"
#define XmNdeviceNameColour "deviceNameColour"
#define XmCDeviceNameColour "DeviceNameColour"
#define XmNdrawingBackgroundColour "drawingBackgroundColour"
#define XmCDrawingBackgroundColour "DrawingBackgroundColour"
#define XmNfibreColour "fibreColour"
#define XmCFibreColour "FibreColour"
#define XmNhighlightedColour "highlightedColour"
#define XmCHighlightedColour "HighlightedColour"
#define XmNlandmarkColour "landmarkColour"
#define XmCLandmarkColour "LandmarkColour"
#define XmNmaintainMapAspectRatio "maintainMapAspectRatio"
#define XmCMaintainMapAspectRatio "MaintainMapAspectRatio"
#define XmNmapBackgroundColour "mapBackgroundColour"
#define XmCMapBackgroundColour "MapBackgroundColour"
#define XmNpixelsBetweenContourValues "pixelsBetweenContourValues"
#define XmCPixelsBetweenContourValues "PixelsBetweenContourValues"
#define XmNunhighlightedColour "unhighlightedColour"
#define XmCUnhighlightedColour "UnhighlightedColour"
	static XtResource background_colour_default_resource[]=
	{
		{
			XmNdrawingBackgroundColour,
			XmCDrawingBackgroundColour,
			XmRString,
			sizeof(String),
			0,
			XmRString,
			"lightgray"
		},
	};
	static XtResource background_colour_resource[]=
	{
		{
			XmNmapBackgroundColour,
			XmCMapBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Map_drawing_information_settings,background_drawing_colour),
			XmRString,
			(String)NULL
		},
	};
	static XtResource resources[]=
	{
		{
			XmNcolourBarTextColour,
			XmCColourBarTextColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Map_drawing_information_settings,spectrum_text_colour),
			XmRString,
			"yellow"
		},
		{
			XmNcontourColour,
			XmCContourColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Map_drawing_information_settings,contour_colour),
			XmRString,
			"black"
		},
		{
			XmNdeviceNameColour,
			XmCDeviceNameColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Map_drawing_information_settings,node_text_colour),
			XmRString,
			"yellow"
		},
		{
			XmNfibreColour,
			XmCFibreColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Map_drawing_information_settings,fibre_colour),
			XmRString,
			"black"
		},
		{
			XmNhighlightedColour,
			XmCHighlightedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Map_drawing_information_settings,highlighted_colour),
			XmRString,
			"white"
		},
		{
			XmNlandmarkColour,
			XmCLandmarkColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Map_drawing_information_settings,landmark_colour),
			XmRString,
			"black"
		},
		{
			XmNmaintainMapAspectRatio,
			XmCMaintainMapAspectRatio,
			XmRBoolean,
			sizeof(Boolean),
			XtOffsetOf(Map_drawing_information_settings,maintain_aspect_ratio),
			XmRString,
			"false"
		},
		{
			XmNpixelsBetweenContourValues,
			XmCPixelsBetweenContourValues,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Map_drawing_information_settings,
				pixels_between_contour_values),
			XmRString,
			"100"
		},
		{
			XmNunhighlightedColour,
			XmCUnhighlightedColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Map_drawing_information_settings,unhighlighted_colour),
			XmRString,
			"red"
		},
	};
	unsigned long mask,plane_masks[1];
	Visual *visual;
	XColor *spectrum_rgb;
	XGCValues values;
	XVisualInfo *visual_info,visual_info_template;

	ENTER(create_Map_drawing_information);
	/* check arguments */
	if ((user_interface)
#if defined (UNEMAP_USE_3D)
			&&element_point_ranges_selection&&
			element_selection&&node_selection&&data_selection&&texture_manager&&
			interactive_tool_manager&&scene_manager&&light_model_manager&&
			light_manager&&spectrum_manager&&graphical_material_manager&&
			data_manager&&glyph_list&&graphical_material&&
			computed_field_package&&light&&light_model
#endif /* defined (UNEMAP_USE_3D) */
		)
	{
		if (ALLOCATE(map_drawing_information,struct Map_drawing_information,1))
		{
			display=User_interface_get_display(user_interface);
			screen_number=XDefaultScreen(display);
			/* the drawable has to have the correct depth and screen */
			XtVaGetValues(User_interface_get_application_shell(user_interface),XmNdepth,&depth,NULL);
			depth_screen_drawable=XCreatePixmap(User_interface_get_display(user_interface),
				XRootWindow(User_interface_get_display(user_interface),screen_number),1,1,depth);
			colour_map=XDefaultColormap(display,screen_number);
			visual=XDefaultVisual(display,screen_number);
#if defined (TEST_TRUE_COLOUR_VISUAL)
/*???debug.  To test true colour visuals */
colour_map=default_colour_map;
visual=default_visual;
#endif /* defined (TEST_TRUE_COLOUR_VISUAL) */
			map_drawing_information->user_interface=user_interface;
			map_drawing_information->font=User_interface_get_normal_font(user_interface);
			map_drawing_information->number_of_spectrum_colours=MAX_SPECTRUM_COLOURS;
			map_drawing_information->colour_map=colour_map;
			map_drawing_information->spectrum=CREATE(Spectrum)("mapping_spectrum");
#if defined (UNEMAP_USE_3D)
			map_drawing_information->torso_arm_labels=(struct GT_object *)NULL;/*FOR AJP*/
			map_drawing_information->element_point_ranges_selection=
				element_point_ranges_selection;
			map_drawing_information->element_selection=element_selection;
			map_drawing_information->node_selection=node_selection;
			map_drawing_information->data_selection=data_selection;
			/* for the cmgui graphics window */
			map_drawing_information->viewed_scene=0;
			/*set to 1 so that will initially regenerate */
			map_drawing_information->electrodes_accepted_or_rejected=1;
			map_drawing_information->no_interpolation_colour=create_Colour(0.65,0.65,0.65);
			map_drawing_information->background_colour=create_Colour(0,0,0);
			map_drawing_information->light=(struct Light *)NULL;
			map_drawing_information->light_model=(struct Light_model *)NULL;
			map_drawing_information->scene=(struct Scene *)NULL;
			map_drawing_information->scene_viewer=(struct Scene_viewer *)NULL;
			map_drawing_information->user_interface=(struct User_interface *)NULL;
			map_drawing_information->computed_field_package=(struct Computed_field_package *)NULL;
			map_drawing_information->time_keeper = (struct Time_keeper *)NULL;
			map_drawing_information->map_graphical_material=(struct Graphical_material *)NULL;
			/* create an electrode material*/
			if (map_drawing_information->electrode_graphical_material=
				CREATE(Graphical_material)("electrode"))
			{
				if (map_drawing_information->electrode_colour =create_Colour(1,0,0))
				{
					Graphical_material_set_ambient(
						map_drawing_information->electrode_graphical_material,
						map_drawing_information->electrode_colour);
					Graphical_material_set_diffuse(
						map_drawing_information->electrode_graphical_material,
						map_drawing_information->electrode_colour);
					ACCESS(Graphical_material)
						(map_drawing_information->electrode_graphical_material);
					if (!ADD_OBJECT_TO_MANAGER(Graphical_material)
						(map_drawing_information->electrode_graphical_material,
						graphical_material_manager))
					{
						DEACCESS(Graphical_material)
						(&map_drawing_information->electrode_graphical_material);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Map_drawing_information.  failed to create electrode_colour");
					map_drawing_information->electrode_graphical_material=
					(struct Graphical_material *)NULL;
					map_drawing_information->electrode_colour=(struct Colour *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Map_drawing_information. failed to createelectrode_graphical_material");
				map_drawing_information->electrode_graphical_material=
				(struct Graphical_material *)NULL;
			}
			map_drawing_information->light_manager=light_manager;
			map_drawing_information->texture_manager=texture_manager;
			map_drawing_information->interactive_tool_manager=interactive_tool_manager;
			map_drawing_information->scene_manager=scene_manager;
			map_drawing_information->light_model_manager=light_model_manager;
			map_drawing_information->spectrum_manager=spectrum_manager;
			map_drawing_information->graphical_material_manager=graphical_material_manager;
			map_drawing_information->data_manager=data_manager;
			map_drawing_information->glyph_list=glyph_list; /* like a manager, so don't access*/
			set_map_drawing_information_map_graphical_material(map_drawing_information,
				graphical_material);
			set_map_drawing_information_computed_field_package(map_drawing_information,
				computed_field_package);
			set_map_drawing_information_light(map_drawing_information,light);
			set_map_drawing_information_light_model(map_drawing_information,
				light_model);
			if (!ADD_OBJECT_TO_MANAGER(Spectrum)(map_drawing_information->spectrum,
				map_drawing_information->spectrum_manager))

			{
				display_message(ERROR_MESSAGE,"create_Map_drawing_information. "
				" Could not add spectrum to manager");
			}
#endif /* defined (UNEMAP_USE_3D) */
			if (visual_info=XGetVisualInfo(display,VisualNoMask,&visual_info_template,
				&number_of_visuals))
			{
				i=0;
				while ((i<number_of_visuals)&&(visual!=(visual_info[i]).visual))
				{
					i++;
				}
				if (visual==(visual_info[i]).visual)
				{
					switch ((visual_info[i]).class)
					{
						case DirectColor: case PseudoColor: case GrayScale:
						{
							map_drawing_information->read_only_colour_map=0;
#if defined (DEBUG)
/*???debug */
printf("read/write colour map\n");
#endif /* defined (DEBUG) */
						} break;
						default:
						{
							map_drawing_information->read_only_colour_map=1;
#if defined (DEBUG)
/*???debug */
printf("read only colour map\n");
#endif /* defined (DEBUG) */
						} break;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
		"create_Map_drawing_information.  Could not find info for default visual");
					map_drawing_information->read_only_colour_map=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Map_drawing_information.  Error getting visual info");
				map_drawing_information->read_only_colour_map=1;
			}
			map_drawing_information->spectrum_colours=(Pixel *)NULL;
			/* retrieve_settings */
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&((background_colour_resource[0]).default_addr),
				background_colour_default_resource,
				XtNumber(background_colour_default_resource),NULL);
			/* NB.  XtVaGetApplicationResources does not allocate memory for the
				default colour String, so it does not need to be free'd */
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				map_drawing_information,background_colour_resource,
				XtNumber(background_colour_resource),NULL);
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				map_drawing_information,resources,XtNumber(resources),NULL);
			/* create the graphics contexts */
			mask=GCLineStyle|GCBackground|GCFont|GCForeground|GCFunction;
			values.font=User_interface_get_normal_font(user_interface)->fid;
			values.line_style=LineSolid;
			values.background=map_drawing_information->background_drawing_colour;
			values.foreground=map_drawing_information->background_drawing_colour;
			values.function=GXcopy;
			(map_drawing_information->graphics_context).background_drawing_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=map_drawing_information->contour_colour;
			(map_drawing_information->graphics_context).contour_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			(map_drawing_information->graphics_context).copy=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=map_drawing_information->fibre_colour;
			(map_drawing_information->graphics_context).fibre_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=map_drawing_information->highlighted_colour;
			(map_drawing_information->graphics_context).highlighted_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=map_drawing_information->node_text_colour;
			(map_drawing_information->graphics_context).node_marker_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=map_drawing_information->node_text_colour;
			(map_drawing_information->graphics_context).node_text_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			(map_drawing_information->graphics_context).spectrum=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=map_drawing_information->spectrum_text_colour^
				map_drawing_information->background_drawing_colour;
			values.function=GXxor;
			(map_drawing_information->graphics_context).spectrum_marker_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=map_drawing_information->spectrum_text_colour;
			values.function=GXcopy;
			(map_drawing_information->graphics_context).spectrum_text_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			values.foreground=map_drawing_information->unhighlighted_colour;
			(map_drawing_information->graphics_context).unhighlighted_colour=
				XCreateGC(display,depth_screen_drawable,mask,&values);
			XFreePixmap(User_interface_get_display(user_interface),depth_screen_drawable);
			number_of_spectrum_colours=MAX_SPECTRUM_COLOURS;
			ALLOCATE(spectrum_colours,Pixel,number_of_spectrum_colours);
			ALLOCATE(spectrum_rgb,XColor,number_of_spectrum_colours);
			if (spectrum_colours&&spectrum_rgb)
			{
				map_drawing_information->spectrum_colours=spectrum_colours;
				map_drawing_information->spectrum_rgb=spectrum_rgb;
				if (map_drawing_information->read_only_colour_map)
				{
					map_drawing_information->boundary_colour=(Pixel)NULL;
					for (i=number_of_spectrum_colours;i>0;i--)
					{
						*spectrum_colours=(Pixel)NULL;
						spectrum_colours++;
					}
				}
				else
				{
					/* allocate the map boundary pixel */
					if (XAllocColorCells(display,colour_map,False,plane_masks,0,
						&(map_drawing_information->boundary_colour),1))
					{
						/* see how many colour cells can be allocated */
							/*???DB.  Want to do this as late as possible so that the widgets
								etc have a chance to get their colours */
						while ((number_of_spectrum_colours>0)&&
							(!XAllocColorCells(display,colour_map,False,plane_masks,0,
							spectrum_colours,number_of_spectrum_colours)))
						{
							number_of_spectrum_colours--;
						}
						/* give back a few for late allocation */
						if (number_of_spectrum_colours>10)
						{
							number_of_spectrum_colours -= 10;
							XFreeColors(display,colour_map,
								spectrum_colours+number_of_spectrum_colours,10,0);
						}
						else
						{
							XFreeColors(display,colour_map,spectrum_colours,
								number_of_spectrum_colours,0);
							display_message(ERROR_MESSAGE,
				"create_Map_drawing_information.  Could not allocate spectrum.  %d %d",
								number_of_spectrum_colours,MAX_SPECTRUM_COLOURS);
							number_of_spectrum_colours=0;
						}
					}
					else
					{
						number_of_spectrum_colours=0;
						map_drawing_information->boundary_colour=(Pixel)NULL;
						display_message(ERROR_MESSAGE,"create_Map_drawing_information.  "
							"Could not allocate map boundary colour");
					}
				}
			}
			else
			{
				number_of_spectrum_colours=0;
				DEALLOCATE(spectrum_colours);
				DEALLOCATE(spectrum_rgb);
				display_message(ERROR_MESSAGE,
				"create_Map_drawing_information.  Could not allocate spectrum memory");
			}
			map_drawing_information->number_of_spectrum_colours=
				number_of_spectrum_colours;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Map_drawing_information.  Could not allocate memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Map_drawing_information.  Missing user_interface");
		map_drawing_information=(struct Map_drawing_information *)NULL;
	}
	LEAVE;

	return (map_drawing_information);
} /* create_Map_drawing_information */

int destroy_Map_drawing_information(
	struct Map_drawing_information **map_drawing_information_address)
/*******************************************************************************
LAST MODIFIED : 10 July 1998

DESCRIPTION :
destroys the map_drawing_information
==============================================================================*/
{
	Display *display;
	int return_code;
	struct Map_drawing_information *map_drawing_information;
	unsigned long planes;

	ENTER(destroy_Map_drawing_information);
	if (map_drawing_information_address&&
		(map_drawing_information= *map_drawing_information_address)&&
		(map_drawing_information->user_interface))
	{		
		display=User_interface_get_display(map_drawing_information->user_interface);
		XFreeGC(display,(map_drawing_information->graphics_context).
			background_drawing_colour);
		XFreeGC(display,(map_drawing_information->graphics_context).contour_colour);
		XFreeGC(display,(map_drawing_information->graphics_context).copy);
		XFreeGC(display,(map_drawing_information->graphics_context).fibre_colour);
		XFreeGC(display,(map_drawing_information->graphics_context).
			highlighted_colour);
		XFreeGC(display,(map_drawing_information->graphics_context).
			node_marker_colour);
		XFreeGC(display,(map_drawing_information->graphics_context).
			node_text_colour);
		XFreeGC(display,(map_drawing_information->graphics_context).spectrum);
		XFreeGC(display,(map_drawing_information->graphics_context).
			spectrum_marker_colour);
		XFreeGC(display,(map_drawing_information->graphics_context).
			spectrum_text_colour);
		XFreeGC(display,(map_drawing_information->graphics_context).
			unhighlighted_colour);
		planes=0;
		XFreeColors(display,map_drawing_information->colour_map,
			&(map_drawing_information->boundary_colour),1,planes);
		XFreeColors(display,map_drawing_information->colour_map,
			map_drawing_information->spectrum_colours,
			map_drawing_information->number_of_spectrum_colours,planes);
		DEALLOCATE(map_drawing_information->spectrum_colours);
		DEALLOCATE(map_drawing_information->spectrum_rgb);
#if defined (UNEMAP_USE_3D)
		destroy_Colour(&map_drawing_information->no_interpolation_colour);
		destroy_Colour(&map_drawing_information->background_colour);
		destroy_Colour(&map_drawing_information->electrode_colour);
		DEACCESS(Light)(&map_drawing_information->light);
		DEACCESS(Light_model)(&map_drawing_information->light_model);
		/* creation/destruction handled elsewhere. No access count to access*/
		map_drawing_information->scene_viewer=(struct Scene_viewer *)NULL;
		DEACCESS(Scene)(&map_drawing_information->scene);
		DEACCESS(Graphical_material)(&map_drawing_information->map_graphical_material);
		DEACCESS(Graphical_material)(&map_drawing_information->electrode_graphical_material);
		DEACCESS(Time_keeper)(&map_drawing_information->time_keeper);
		/* torso_arm_labels*/
		DEACCESS(GT_object)(&(map_drawing_information->torso_arm_labels));/*FOR AJP*/
#endif /* defined (UNEMAP_USE_3D) */
		DEALLOCATE(*map_drawing_information_address);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Map_drawing_information */

int get_map_drawing_information_electrodes_accepted_or_rejected(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :13 December  2000

DESCRIPTION :
gets the electrodes_accepted_or_rejected flag of the <map_drawing_information>
==============================================================================*/
{
	int electrodes_accepted_or_rejected;

	ENTER(get_map_drawing_information_electrodes_accepted_or_rejected)
	if (map_drawing_information)
	{
		electrodes_accepted_or_rejected=map_drawing_information->electrodes_accepted_or_rejected;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_map_drawing_information_electrodes_accepted_or_rejected. invalid arguments");
		electrodes_accepted_or_rejected=0;
	}
	LEAVE;
	
	return (electrodes_accepted_or_rejected);
}/* get_map_drawing_information_electrodes_accepted_or_rejected */

int set_map_drawing_information_electrodes_accepted_or_rejected(
struct Map_drawing_information *map_drawing_information,int accep_rej)
/*******************************************************************************
LAST MODIFIED :  13 December 2000

DESCRIPTION :
sets the electrodes_accepted_or_rejected flag of the <map_drawing_information>
to 1 if <accep_rej> >0,
0 if <accep_rej> = 0.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_electrodes_accepted_or_rejected)
	if (map_drawing_information&&(accep_rej>-1))
	{
		if (accep_rej)
		{
			/* scene has been viewed */
			map_drawing_information->electrodes_accepted_or_rejected=1;
		}
		else
		{
			/* scene hasn't been viewed */
			map_drawing_information->electrodes_accepted_or_rejected=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_electrodes_accepted_or_rejected."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_drawing_information_electrodes_accepted_or_rejected */

#if defined (UNEMAP_USE_3D)
DECLARE_OBJECT_FUNCTIONS(Map_3d_package)

struct Map_3d_package *CREATE(Map_3d_package)(
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(GROUP(FE_element))	*element_group_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 8 September 1999

DESCRIPTION:
Create and  and set it's components
==============================================================================*/
{
	struct Map_3d_package *map_3d_package;

	ENTER(CREATE(Map_3d_package));
	map_3d_package=(struct Map_3d_package *)NULL;
	if (fe_field_manager&&element_group_manager&&data_manager&&data_group_manager&&
		node_manager&&node_group_manager&&element_manager&&computed_field_manager)
	{
		if (ALLOCATE(map_3d_package,struct Map_3d_package,1))
		{
			map_3d_package->number_of_map_columns=0;
			map_3d_package->number_of_map_rows=0;
			map_3d_package->fit_name=(char *)NULL;
			map_3d_package->node_order_info=(struct FE_node_order_info *)NULL;
			map_3d_package->map_position_field=(struct FE_field *)NULL;
			map_3d_package->map_fit_field=(struct FE_field *)NULL;
			map_3d_package->number_of_contours=0;
			map_3d_package->contour_settings=(struct GT_element_settings **)NULL;
			map_3d_package->fe_field_manager=fe_field_manager;
			map_3d_package->node_group=(struct GROUP(FE_node) *)NULL;
			map_3d_package->element_group=(struct GROUP(FE_element) *)NULL;
			map_3d_package->element_group_manager=element_group_manager;
			map_3d_package->data_manager=data_manager;
			map_3d_package->node_manager=node_manager;
			map_3d_package->data_group_manager=data_group_manager;
			map_3d_package->node_group_manager=node_group_manager;
			map_3d_package->element_manager=element_manager;
			map_3d_package->computed_field_manager=computed_field_manager;
			map_3d_package->electrode_glyph=(struct GT_object *)NULL;
			map_3d_package->electrode_size=0;
			map_3d_package->electrodes_label_type=HIDE_ELECTRODE_LABELS;
			map_3d_package->colour_electrodes_with_signal=1;
			map_3d_package->electrodes_min_z=0;
			map_3d_package->electrodes_max_z=0;
			map_3d_package->mapped_torso_node_group=(struct GROUP(FE_node) *)NULL;
			map_3d_package->mapped_torso_element_group=(struct GROUP(FE_element) *)NULL;
			map_3d_package->delaunay_torso_node_group=(struct GROUP(FE_node) *)NULL;
			map_3d_package->delaunay_torso_element_group=(struct GROUP(FE_element) *)NULL;
			map_3d_package->access_count=0.0;

		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Map_3d_package).  Not enough memory");
			if (map_3d_package)
			{
				DEALLOCATE(map_3d_package);
			}
		}
	}
	LEAVE;
	
	return (map_3d_package);
} /* CREATE(Map_3d_package) */

int free_map_3d_package_map_contours(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Frees the array of map contour GT_element_settings stored in the <map_3d_package>
==============================================================================*/
{
	int i,number_of_contours,return_code;

	ENTER(free_map_3d_package_map_contours);
	if (map_3d_package)
	{
		return_code=1;
		number_of_contours=map_3d_package->number_of_contours;
		if (number_of_contours)
		{
			for(i=0;i<number_of_contours;i++)
			{
				DEACCESS(GT_element_settings)(&(map_3d_package->contour_settings[i]));
			}
			DEALLOCATE(map_3d_package->contour_settings);
			map_3d_package->contour_settings=(struct GT_element_settings **)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"free_map_3d_package_map_contours Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/*free_map_3d_package_map_contours */

int DESTROY(Map_3d_package)(struct Map_3d_package **map_3d_package_address)
/*******************************************************************************
LAST MODIFIED : 18 January 2002

DESCRIPTION :
Frees the memory for the Map_3d_package and sets <*package_address>
to NULL.
==============================================================================*/
{
	char *group_name;
	int return_code;
	struct FE_field *temp_field;
	struct Map_3d_package *map_3d_package;
	struct GROUP(FE_element) *map_element_group,*temp_map_element_group;
	struct GROUP(FE_node) *temp_map_node_group,*data_group;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(FE_node) *node_manager,*data_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(DESTROY(Map_3d_package));
	group_name=(char *)NULL;
	if ((map_3d_package_address)&&(map_3d_package= *map_3d_package_address))
	{
		element_manager=map_3d_package->element_manager;
		element_group_manager=map_3d_package->element_group_manager;
		node_manager=map_3d_package->node_manager;
		data_manager=map_3d_package->data_manager;
		data_group_manager=map_3d_package->data_group_manager;
		node_group_manager=map_3d_package->node_group_manager;
		fe_field_manager=map_3d_package->fe_field_manager;
		computed_field_manager=map_3d_package->computed_field_manager;
		return_code=1;
		/* electrode_glyph*/
		DEACCESS(GT_object)(&(map_3d_package->electrode_glyph));
		/* node_order_info */
		DEACCESS(FE_node_order_info)(&(map_3d_package->node_order_info));
		/* contours*/
		free_map_3d_package_map_contours(map_3d_package);
		/* map_group */
		map_element_group=map_3d_package->element_group;
		/* deaccess map_element_group. map_node_group deaccessed in */
		/*free_node_and_element_and_data_groups*/
		if (map_element_group)
		{
			temp_map_element_group=map_element_group;
			DEACCESS(GROUP(FE_element))(&temp_map_element_group);
			free_node_and_element_and_data_groups(&(map_3d_package->node_group),
				element_manager,element_group_manager,data_manager,
				data_group_manager,node_manager,node_group_manager);
		}
		/* mapped torso groups. The contained nodes and elements will remain, in */
		/* the default_torso_groups, whose name is stored in the unemap_package->default_torso_name*/
		/* mapped_torso_element_group,mapped_torso_node_group accesssed by map_3d_package, but */
		/* corresponding data group isn't */
		temp_map_element_group=map_3d_package->mapped_torso_element_group;
		if (temp_map_element_group)
		{
			GET_NAME(GROUP(FE_element))(temp_map_element_group,&group_name);
			DEACCESS(GROUP(FE_element))(&temp_map_element_group);
			if (MANAGED_OBJECT_NOT_IN_USE(GROUP(FE_element))(
				map_3d_package->mapped_torso_element_group, element_group_manager))
			{
				REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))
					(map_3d_package->mapped_torso_element_group,
						element_group_manager);
			}
		}
		temp_map_node_group=map_3d_package->mapped_torso_node_group;
		if (temp_map_node_group)
		{
			DEACCESS(GROUP(FE_node))(&temp_map_node_group);

			if (MANAGED_OBJECT_NOT_IN_USE(GROUP(FE_node))(
				map_3d_package->mapped_torso_node_group, node_group_manager))
			{
				REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))
					(map_3d_package->mapped_torso_node_group,
						node_group_manager);
			}
		}

		map_element_group=map_3d_package->delaunay_torso_element_group;
		if (map_element_group)
		{
			/* deaccess delaunay_torso_element_group. delaunay_torso_node_group deaccessed in */
			/*free_node_and_element_and_data_groups*/
			temp_map_element_group=map_element_group;
			DEACCESS(GROUP(FE_element))(&temp_map_element_group);
			/* this will destroy the elements, but not the nodes, as they're accessed elsewhere*/
			free_node_and_element_and_data_groups(&(map_3d_package->delaunay_torso_node_group),
				element_manager,element_group_manager,data_manager,
				data_group_manager,node_manager,node_group_manager);
		}
		if (group_name&&(data_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)
			(group_name,data_group_manager)))
		{
			if (MANAGED_OBJECT_NOT_IN_USE(GROUP(FE_node))(
				data_group, data_group_manager))
			{
				REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(data_group,
					data_group_manager);
			}
		}
		DEALLOCATE(group_name);
		/* computed_fields */
		if (map_3d_package->map_fit_field)
		{
			remove_computed_field_from_manager_given_FE_field(
				computed_field_manager,map_3d_package->map_fit_field);
		}
		if (map_3d_package->map_position_field)
		{
			remove_computed_field_from_manager_given_FE_field(computed_field_manager,
				map_3d_package->map_position_field);
		}
		/* map_fit_field */
		if (map_3d_package->map_fit_field)
		{
			temp_field=map_3d_package->map_fit_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				map_3d_package->map_fit_field);
			map_3d_package->map_fit_field=(struct FE_field *)NULL;
		}
		/* map_position_field */
		if (map_3d_package->map_position_field)
		{
			temp_field=map_3d_package->map_position_field;
			DEACCESS(FE_field)(&temp_field);
			destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				map_3d_package->map_position_field);
			map_3d_package->map_position_field=(struct FE_field *)NULL;
		}
		DEALLOCATE(map_3d_package->contour_settings);
		/* fit_name */
		DEALLOCATE(map_3d_package->fit_name);
		DEALLOCATE(*map_3d_package_address);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(map_3d_package) */

struct GT_object *get_map_3d_package_electrode_glyph(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the map_electrode_glyph for map_3d_package
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/
{
	struct GT_object *electrode_glyph;

	ENTER(get_map_3d_package_electrode_glyph);
	if (map_3d_package)
	{
		electrode_glyph=map_3d_package->electrode_glyph;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_electrode_glyph."
			" invalid arguments");
		electrode_glyph=(struct GT_object *)NULL;
	}
	LEAVE;
	
	return (electrode_glyph);
} /* get_map_3d_package_electrode_glyph */

int set_map_3d_package_electrode_glyph(struct Map_3d_package *map_3d_package,
	struct GT_object *electrode_glyph)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Sets the <electrode_glyph>  for <map_3d_package>
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_electrode_glyph);
	if (map_3d_package)
	{
		return_code =1;
		REACCESS(GT_object)
			(&(map_3d_package->electrode_glyph),electrode_glyph);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_electrode_glyph ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_electrode_glyph */

FE_value get_map_3d_package_electrode_size(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
gets the map_electrode_size for map_3d_package <map_number>
==============================================================================*/
{
	FE_value electrode_size;

	ENTER(get_map_3d_package_electrode_size);
	if (map_3d_package)
	{
		electrode_size=map_3d_package->electrode_size;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_electrode_size."
			" invalid arguments");
		electrode_size=0;
	}
	LEAVE;
	
	return (electrode_size);
} /* get_map_3d_package_electrode_size */

int set_map_3d_package_electrode_size(struct Map_3d_package *map_3d_package,
	FE_value electrode_size)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Sets the electrode_size  for map_3d_package <map_number>
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_electrode_size);
	if (map_3d_package)
	{
		return_code =1;
		map_3d_package->electrode_size=electrode_size;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_electrode_size ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_electrode_size */

FE_value get_map_3d_package_electrodes_min_z(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 9 November 2000

DESCRIPTION :
gets the map_electrodes_min_z for map_3d_package <map_number>
==============================================================================*/
{
	FE_value electrodes_min_z;

	ENTER(get_map_3d_package_electrodes_min_z);
	if (map_3d_package)
	{
		electrodes_min_z=map_3d_package->electrodes_min_z;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_electrodes_min_z."
			" invalid arguments");
		electrodes_min_z=0;
	}
	LEAVE;
	
	return (electrodes_min_z);
} /* get_map_3d_package_electrodes_min_z */

int set_map_3d_package_electrodes_min_z(struct Map_3d_package *map_3d_package,
	FE_value electrodes_min_z)
/*******************************************************************************
LAST MODIFIED : 9 November 2000

DESCRIPTION :
Sets the electrodes_min_z  for map_3d_package <map_number>
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_electrodes_min_z);
	if (map_3d_package)
	{
		return_code =1;
		map_3d_package->electrodes_min_z=electrodes_min_z;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_electrodes_min_z ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_electrodes_min_z */

FE_value get_map_3d_package_electrodes_max_z(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 9 November 2000

DESCRIPTION :
gets the map_electrodes_max_z for map_3d_package <map_number>
==============================================================================*/
{
	FE_value electrodes_max_z;

	ENTER(get_map_3d_package_electrodes_max_z);
	if (map_3d_package)
	{
		electrodes_max_z=map_3d_package->electrodes_max_z;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_electrodes_max_z."
			" invalid arguments");
		electrodes_max_z=0;
	}
	LEAVE;
	
	return (electrodes_max_z);
} /* get_map_3d_package_electrodes_max_z */

int set_map_3d_package_electrodes_max_z(struct Map_3d_package *map_3d_package,
	FE_value electrodes_max_z)
/*******************************************************************************
LAST MODIFIED :  November 2000

DESCRIPTION :
Sets the electrodes_max_z  for map_3d_package <map_number>
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_electrodes_max_z);
	if (map_3d_package)
	{
		return_code =1;
		map_3d_package->electrodes_max_z=electrodes_max_z;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_electrodes_max_z ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_electrodes_max_z */

enum Electrodes_label_type get_map_3d_package_electrodes_label_type(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
gets the map_electrodes_label_type for map_3d_package <map_number>
==============================================================================*/
{
	enum Electrodes_label_type electrodes_label_type;

	ENTER(get_map_3d_package_electrodes_label_type);
	if (map_3d_package)
	{
		electrodes_label_type=map_3d_package->electrodes_label_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_electrodes_label_type."
			" invalid arguments");
		electrodes_label_type=HIDE_ELECTRODE_LABELS;
	}
	LEAVE;
	
	return (electrodes_label_type);
} /* get_map_3d_package_electrodes_label_type */

int set_map_3d_package_electrodes_label_type(struct Map_3d_package *map_3d_package,
	enum Electrodes_label_type electrodes_label_type)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Sets the electrodes_label_type  for map_3d_package <map_number>
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_electrodes_label_type);
	if (map_3d_package)
	{
		return_code =1;
		map_3d_package->electrodes_label_type=electrodes_label_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_electrodes_label_type ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_electrodes_label_type */

int get_map_3d_package_colour_electrodes_with_signal(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the map_colour_electrodes_with_signal for map_3d_package
==============================================================================*/
{
	int colour_electrodes_with_signal;

	ENTER(get_map_3d_package_colour_electrodes_with_signal);
	if (map_3d_package)
	{
		colour_electrodes_with_signal=
			map_3d_package->colour_electrodes_with_signal;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_colour_electrodes_with_signal."
			" invalid arguments");
		colour_electrodes_with_signal=HIDE_ELECTRODE_LABELS;
	}
	LEAVE;
	
	return (colour_electrodes_with_signal);
} /* get_map_3d_package_colour_electrodes_with_signal */

int set_map_3d_package_colour_electrodes_with_signal(
	struct Map_3d_package *map_3d_package,int colour_electrodes_with_signal)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Sets the colour_electrodes_with_signal  for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_colour_electrodes_with_signal);
	if (map_3d_package)
	{
		return_code =1;
		map_3d_package->colour_electrodes_with_signal=
			colour_electrodes_with_signal;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_map_3d_package_colour_electrodes_with_signal ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_colour_electrodes_with_signal */

int get_map_3d_package_number_of_map_rows(	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the number_of_map_rows for map_3d_package
Returns -1 on error
==============================================================================*/
{
	int number_of_map_rows;

	ENTER(get_map_3d_package_number_of_map_rows);
	if (map_3d_package)
	{
		number_of_map_rows=map_3d_package->number_of_map_rows;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_number_of_map_rows."
			" invalid arguments");
		number_of_map_rows=-1;
	}
	LEAVE;
	
	return (number_of_map_rows);
} /* get_map_3d_package_number_of_map_rows */

int set_map_3d_package_number_of_map_rows(struct Map_3d_package *map_3d_package,
	int number_of_map_rows)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
sets the number_of_map_rows for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_number_of_map_rows);
	if (map_3d_package)
	{
		return_code=1;
		map_3d_package->number_of_map_rows=number_of_map_rows;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_number_of_map_rows."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_number_of_map_rows */

int get_map_3d_package_number_of_map_columns(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the number_of_map_columns for map_3d_package
Returns -1 on error
==============================================================================*/
{
	int number_of_map_columns;

	ENTER(get_map_3d_package_number_of_map_columns);
	if (map_3d_package)
	{
		number_of_map_columns=map_3d_package->number_of_map_columns;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_number_of_map_columns."
				" invalid arguments");
		number_of_map_columns=-1;
	}
	LEAVE;
	
	return (number_of_map_columns);
} /* get_map_3d_package_number_of_map_columns */

int set_map_3d_package_number_of_map_columns(struct Map_3d_package *map_3d_package,
	int number_of_map_columns)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
sets the number_of_map_columns for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_number_of_map_columns);
	if (map_3d_package)
	{
		return_code=1;
		map_3d_package->number_of_map_columns=number_of_map_columns;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_number_of_map_columns."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_number_of_map_columns */

int get_map_3d_package_contours(struct Map_3d_package *map_3d_package,
	int *number_of_contours,struct GT_element_settings ***contour_settings)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the <number_of_contours> and <contour_settings> for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(get_map_3d_package_contours);
	return_code=1;
	if (map_3d_package)
	{
		*number_of_contours=map_3d_package->number_of_contours;
		*contour_settings=map_3d_package->contour_settings;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_contours."
			" invalid arguments");
		return_code=0; /* No map_3d_package */
		*number_of_contours=0;
		*contour_settings=(struct GT_element_settings **)NULL;
	}
	LEAVE;
	
	return (return_code);
} /* get_map_3d_package_contours */

int set_map_3d_package_contours(struct Map_3d_package *map_3d_package,
	int number_of_contours,struct GT_element_settings **contour_settings)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
sets the <number_of_contours> and <contour_settings> for map_3d_package
==============================================================================*/
{
	int i,return_code;

	ENTER(set_map_3d_package_contours)
	return_code=0;
	if (map_3d_package)
	{

		return_code=1;
		map_3d_package->number_of_contours=number_of_contours;
		map_3d_package->contour_settings=contour_settings;
		for(i=0;i<number_of_contours;i++)
		{
			map_3d_package->contour_settings[i]=
				ACCESS(GT_element_settings)(contour_settings[i]);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_contours."
			" invalid arguments");
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_contours */

char *get_map_3d_package_fit_name(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the fit_name for map_3d_package
==============================================================================*/
{
	char *fit_name;

	ENTER(get_map_3d_package_fit_name);
	if (map_3d_package)
	{
		fit_name=map_3d_package->fit_name;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_fit_name."
				" invalid arguments");
		fit_name=(char *)NULL;
	}
	LEAVE;
	
	return (fit_name);
} /* get_map_3d_package_fit_name */

int set_map_3d_package_fit_name(struct Map_3d_package *map_3d_package,
	char *fit_name)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
sets the fit_name for map_3d_package
==============================================================================*/
{
	int return_code,string_length;

	ENTER(set_map_3d_package_fit_name);
	if (map_3d_package)
	{
		return_code=1;
		string_length = strlen(fit_name);
		string_length++;
		if (ALLOCATE(map_3d_package->fit_name,char,string_length))
		{
			strcpy(map_3d_package->fit_name,fit_name);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_map_3d_package_fit_name."
				"Out of memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_fit_name."
				" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_fit_name */

struct FE_node_order_info *get_map_3d_package_node_order_info(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the node_order_info for map_3d_package
==============================================================================*/
{
	struct FE_node_order_info *node_order_info;

	ENTER(get_map_3d_package_node_order_info);
	if (map_3d_package)
	{
		node_order_info=map_3d_package->node_order_info;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_node_order_info."
				" invalid arguments");
		node_order_info=(struct FE_node_order_info *)NULL;
	}
	LEAVE;
	
	return (node_order_info);
} /* get_map_3d_package_node_order_info */

int set_map_3d_package_node_order_info(struct Map_3d_package *map_3d_package,
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED :8 December 2000

DESCRIPTION :
sets the node_order_info for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_node_order_info);
	if (map_3d_package&&node_order_info)
	{
		REACCESS(FE_node_order_info)
			(&(map_3d_package->node_order_info),node_order_info);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_node_order_info."
				" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_node_order_info */

struct FE_field *get_map_3d_package_position_field(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the map_position_field for map_3d_package
==============================================================================*/
{
	struct FE_field *map_position_field;

	ENTER(get_map_3d_package_position_field);
	if (map_3d_package)
	{
		map_position_field=map_3d_package->map_position_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_position_field."
				" invalid arguments");
		map_position_field=(struct FE_field *)NULL;
	}
	LEAVE;
	
	return (map_position_field);
} /* get_map_3d_package_position_field */

int set_map_3d_package_position_field(struct Map_3d_package *map_3d_package,
	struct FE_field *position_field)
/*******************************************************************************
LAST MODIFIED : December 8 2000

DESCRIPTION :
sets the map_position_field for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_position_field);
	if (map_3d_package)
	{
		return_code=1;
		REACCESS(FE_field)
			(&(map_3d_package->map_position_field),position_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_position_field."
				" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_position_field */

struct FE_field *get_map_3d_package_fit_field(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the map_fit_field for map_3d_package
==============================================================================*/
{
	struct FE_field *map_fit_field;

	ENTER(get_map_3d_package_fit_field);
	if (map_3d_package)
	{
		map_fit_field=map_3d_package->map_fit_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_fit_field."
				" invalid arguments");
		map_fit_field=(struct FE_field *)NULL;
	}
	LEAVE;
	
	return (map_fit_field);
} /* get_map_3d_package_fit_field */

int set_map_3d_package_fit_field(struct Map_3d_package *map_3d_package,
	struct FE_field *fit_field)
/*******************************************************************************
LAST MODIFIED : December 8 2000

DESCRIPTION :
sets the map_fit_field for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_fit_field);
	if (map_3d_package)
	{
		return_code=1;
		REACCESS(FE_field)(&(map_3d_package->map_fit_field),fit_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_fit_field."
				" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_3d_package_fit_field */

struct GROUP(FE_node) *get_map_3d_package_node_group(struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the node_group for map_3d_package
==============================================================================*/
{
	struct GROUP(FE_node) *map_node_group;

	ENTER(get_map_3d_package_node_group);
	if (map_3d_package)
	{
		map_node_group=map_3d_package->node_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_node_group."
			" invalid arguments");
		map_node_group=(struct GROUP(FE_node) *)NULL;
	}
	LEAVE;
	
	return (map_node_group);
}/* get_map_3d_package_node_group */

int set_map_3d_package_node_group(struct Map_3d_package *map_3d_package,
	struct GROUP(FE_node) *map_node_group)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
sets the node_group for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_node_group);
	if (map_3d_package)
	{
		REACCESS(GROUP(FE_node))(&(map_3d_package->node_group),map_node_group);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_node_group."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_3d_package_node_group */

struct GROUP(FE_element) *get_map_3d_package_element_group(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the element_group for map_3d_package
==============================================================================*/
{
	struct GROUP(FE_element) *map_element_group;

	ENTER(get_map_3d_package_element_group);
	if (map_3d_package)
	{
		map_element_group=map_3d_package->element_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_element_group."
			" invalid arguments");
		map_element_group=(struct GROUP(FE_element) *)NULL;
	}
	LEAVE;
	
	return (map_element_group);
}/* get_map_3d_package_element_group */

int set_map_3d_package_element_group(struct Map_3d_package *map_3d_package,
	struct GROUP(FE_element) *map_element_group)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Sets the element_group for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_element_group);
	if (map_3d_package)
	{
		return_code =1;
		REACCESS(GROUP(FE_element))
			(&(map_3d_package->element_group),map_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_element_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_3d_package_element_group */

struct GROUP(FE_element) *get_map_3d_package_mapped_torso_element_group(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the mapped_torso_element_group for map_3d_package
==============================================================================*/
{
	struct GROUP(FE_element) *mapped_torso_element_group;

	ENTER(get_map_3d_package_mapped_torso_element_group);
	if (map_3d_package)
	{
		mapped_torso_element_group=map_3d_package->mapped_torso_element_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_mapped_torso_element_group."
			" invalid arguments");
		mapped_torso_element_group=(struct GROUP(FE_element) *)NULL;
	}
	LEAVE;
	
	return (mapped_torso_element_group);
}/* get_map_3d_package_mapped_torso_element_group */

int set_map_3d_package_mapped_torso_element_group(struct Map_3d_package *map_3d_package,
	struct GROUP(FE_element) *mapped_torso_element_group)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Sets the mapped_torso_element_group for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_mapped_torso_element_group);
	if (map_3d_package)
	{
		return_code =1;
		REACCESS(GROUP(FE_element))
			(&(map_3d_package->mapped_torso_element_group),mapped_torso_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_mapped_torso_element_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_3d_package_mapped_torso_element_group */

struct GROUP(FE_node) *get_map_3d_package_mapped_torso_node_group(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the mapped_torso_node_group for map_3d_package
==============================================================================*/
{
	struct GROUP(FE_node) *mapped_torso_node_group;

	ENTER(get_map_3d_package_mapped_torso_node_group);
	if (map_3d_package)
	{
		mapped_torso_node_group=map_3d_package->mapped_torso_node_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_mapped_torso_node_group."
			" invalid arguments");
		mapped_torso_node_group=(struct GROUP(FE_node) *)NULL;
	}
	LEAVE;
	
	return (mapped_torso_node_group);
}/* get_map_3d_package_mapped_torso_node_group */

int set_map_3d_package_mapped_torso_node_group(struct Map_3d_package *map_3d_package,
	struct GROUP(FE_node) *mapped_torso_node_group)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Sets the mapped_torso_node_group for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_mapped_torso_node_group);
	if (map_3d_package)
	{
		return_code =1;
		REACCESS(GROUP(FE_node))
			(&(map_3d_package->mapped_torso_node_group),mapped_torso_node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_mapped_torso_node_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_3d_package_mapped_torso_node_group */

struct GROUP(FE_element) *get_map_3d_package_delaunay_torso_element_group(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
gets the delaunay_torso_element_group for map_3d_package
==============================================================================*/
{
	struct GROUP(FE_element) *delaunay_torso_element_group;

	ENTER(get_map_3d_package_delaunay_torso_element_group);
	if (map_3d_package)
	{
		delaunay_torso_element_group=map_3d_package->delaunay_torso_element_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_delaunay_torso_element_group."
			" invalid arguments");
		delaunay_torso_element_group=(struct GROUP(FE_element) *)NULL;
	}
	LEAVE;
	
	return (delaunay_torso_element_group);
}/* get_map_3d_package_delaunay_torso_element_group */

int set_map_3d_package_delaunay_torso_element_group(struct Map_3d_package *map_3d_package,
	struct GROUP(FE_element) *delaunay_torso_element_group)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
Sets the delaunay_torso_element_group for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_delaunay_torso_element_group);
	if (map_3d_package)
	{
		return_code =1;
		REACCESS(GROUP(FE_element))
			(&(map_3d_package->delaunay_torso_element_group),delaunay_torso_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_delaunay_torso_element_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_3d_package_delaunay_torso_element_group */

struct GROUP(FE_node) *get_map_3d_package_delaunay_torso_node_group(
	struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
gets the delaunay_torso_node_group for map_3d_package
==============================================================================*/
{
	struct GROUP(FE_node) *delaunay_torso_node_group;

	ENTER(get_map_3d_package_delaunay_torso_node_group);
	if (map_3d_package)
	{
		delaunay_torso_node_group=map_3d_package->delaunay_torso_node_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_3d_package_delaunay_torso_node_group."
			" invalid arguments");
		delaunay_torso_node_group=(struct GROUP(FE_node) *)NULL;
	}
	LEAVE;
	
	return (delaunay_torso_node_group);
}/* get_map_3d_package_delaunay_torso_node_group */

int set_map_3d_package_delaunay_torso_node_group(struct Map_3d_package *map_3d_package,
	struct GROUP(FE_node) *delaunay_torso_node_group)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
Sets the delaunay_torso_node_group for map_3d_package
==============================================================================*/
{
	int return_code;

	ENTER(set_map_3d_package_delaunay_torso_node_group);
	if (map_3d_package)
	{
		return_code =1;
		REACCESS(GROUP(FE_node))
			(&(map_3d_package->delaunay_torso_node_group),delaunay_torso_node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_3d_package_delaunay_torso_node_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_3d_package_delaunay_torso_node_group */

int get_map_drawing_information_viewed_scene(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets the viewed_scene flag of the <map_drawing_information>
==============================================================================*/
{
	int viewed_scene;

	ENTER(get_map_drawing_information_viewed_scene)
	if (map_drawing_information)
	{
		viewed_scene=map_drawing_information->viewed_scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_viewed_scene."
			" invalid arguments");
		viewed_scene=0;
	}
	LEAVE;
	
	return (viewed_scene);
}/* get_map_drawing_information_viewed_scene */

int set_map_drawing_information_viewed_scene(
struct Map_drawing_information *map_drawing_information,int scene_viewed)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
sets the viewed_scene flag of the <map_drawing_information>
to 1 if <scene_viewed> >0,
0 if <scene_viewed> = 0.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_viewed_scene)
	if (map_drawing_information&&(scene_viewed>-1))
	{
		if (scene_viewed)
		{
			/* scene has been viewed */
			map_drawing_information->viewed_scene=1;
		}
		else
		{
			/* scene hasn't been viewed */
			map_drawing_information->viewed_scene=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_viewed_scene."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
}/* set_map_drawing_information_viewed_scene */

struct GT_object *get_map_drawing_information_torso_arm_labels(
	struct Map_drawing_information *drawing_information)/*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 14 December 2000

DESCRIPTION :
gets the map_torso_arm_labels for map_drawing_information <map_number>
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/
{
	struct GT_object *torso_arm_labels;

	ENTER(get_map_drawing_information_torso_arm_labels);
	if (drawing_information)
	{
		torso_arm_labels=drawing_information->torso_arm_labels;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_torso_arm_labels."
			" invalid arguments");
		torso_arm_labels=(struct GT_object *)NULL;
	}
	LEAVE;
	
	return (torso_arm_labels);
} /* get_map_drawing_information_torso_arm_labels */

int set_map_drawing_information_torso_arm_labels(
	struct Map_drawing_information *drawing_information,
	struct GT_object *torso_arm_labels)/*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 14 December 2000

DESCRIPTION :
Sets the torso_arm_labels  for map_drawing_information
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_torso_arm_labels);
	if (drawing_information&&torso_arm_labels)
	{
		return_code =1;
		REACCESS(GT_object)
			(&(drawing_information->torso_arm_labels),torso_arm_labels);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_torso_arm_labels ."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_torso_arm_labels */

struct Colour *get_map_drawing_information_background_colour(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Colour of the <map_drawing_information>.
==============================================================================*/
{
	struct Colour *background_colour;
	ENTER(get_map_drawing_information_background_colour);
	if (map_drawing_information)
	{
		background_colour=map_drawing_information->background_colour;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_background_colour."
				" invalid arguments");
		background_colour = (struct Colour *)NULL;
	}
	LEAVE;
	
	return (background_colour);
} /* get_map_drawing_information_background_colour */

int set_map_drawing_information_background_colour(
	struct Map_drawing_information *map_drawing_information,
	struct Colour *background_colour)
/*******************************************************************************
LAST MODIFIED : 14 July  2000

DESCRIPTION :
Sets the Colour of the <map_drawing_information>.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_background_colour);
	if (map_drawing_information)
	{
		return_code =1;
		map_drawing_information->background_colour->red=background_colour->red;
		map_drawing_information->background_colour->green=background_colour->green;
		map_drawing_information->background_colour->blue=background_colour->blue;

	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_background_colour."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_background_colour */

struct Colour *get_map_drawing_information_no_interpolation_colour(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets the no_interpolation_colour of the <map_drawing_information>.
==============================================================================*/
{
	struct Colour *no_interpolation_colour;

	ENTER(get_map_drawing_information_no_interpolation_colour)
	if (map_drawing_information)
	{
		no_interpolation_colour=map_drawing_information->no_interpolation_colour;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_no_interpolation_colour."
			" invalid arguments");
		no_interpolation_colour=(struct Colour *)NULL;
	}
	LEAVE;
	
	return (no_interpolation_colour);
}/* get_map_drawing_information_no_interpolation_colour */

struct MANAGER(Light) *get_map_drawing_information_Light_manager(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/
{
	struct MANAGER(Light) *light_manager;

	ENTER(get_map_drawing_information_Light_manager);
	if (map_drawing_information)
	{
		light_manager=map_drawing_information->light_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_Light_manager."
			" invalid arguments");
		light_manager = (struct MANAGER(Light) *)NULL;
	}
	LEAVE;
	
	return (light_manager);
}/* get_map_drawing_information_Light_manager */

struct MANAGER(Texture) *get_map_drawing_information_Texture_manager(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/
{
	struct MANAGER(Texture) *texture_manager;

	ENTER(get_map_drawing_information_Texture_manager);
	if (map_drawing_information)
	{
		texture_manager=map_drawing_information->texture_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_Texture_manager."
			" invalid arguments");
		texture_manager = (struct MANAGER(Texture) *)NULL;
	}
	LEAVE;
	
	return (texture_manager);
}/* get_map_drawing_information_Texture_manager */

struct MANAGER(Scene) *get_map_drawing_information_Scene_manager(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/
{
	struct MANAGER(Scene) *scene_manager;

	ENTER(get_map_drawing_information_Scene_manager);
	if (map_drawing_information)
	{
		scene_manager=map_drawing_information->scene_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_Scene_manager."
			" invalid arguments");
		scene_manager = (struct MANAGER(Scene) *)NULL;
	}
	LEAVE;
	
	return (scene_manager);
}/* get_map_drawing_information_Scene_manager */

struct MANAGER(Light_model) *get_map_drawing_information_Light_model_manager(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/
{
	struct MANAGER(Light_model) *light_model_manager;

	ENTER(get_map_drawing_information_Light_model_manager);
	if (map_drawing_information)
	{
		light_model_manager=map_drawing_information->light_model_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_Light_model_manager."
			" invalid arguments");
		light_model_manager = (struct MANAGER(Light_model) *)NULL;
	}
	LEAVE;
	
	return (light_model_manager);
}/* get_map_drawing_information_Light_model_manager */

struct MANAGER(Graphical_material)
		*get_map_drawing_information_Graphical_material_manager(
			struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/
{
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(get_map_drawing_information_Graphical_material_manager);
	if (map_drawing_information)
	{
		graphical_material_manager=map_drawing_information->graphical_material_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_Graphical_material_manager."
			" invalid arguments");
		graphical_material_manager = (struct MANAGER(Graphical_material) *)NULL;
	}
	LEAVE;
	
	return (graphical_material_manager);
}/* get_map_drawing_information_Graphical_material_manager */

struct Light *get_map_drawing_information_light(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Light of the <map_drawing_information>.
==============================================================================*/
{
	struct Light *light;
	ENTER(get_map_drawing_information_light);
	if (map_drawing_information)
	{
		light=map_drawing_information->light;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_light."
				" invalid arguments");
		light = (struct Light *)NULL;
	}
	LEAVE;
	
	return (light);
} /* get_map_drawing_information_light */

int set_map_drawing_information_light(
	struct Map_drawing_information *map_drawing_information,struct Light *light)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Light of the <map_drawing_information>.
==============================================================================*/

{
	int return_code;

	ENTER(set_map_drawing_information_light);
	if (map_drawing_information)
	{
		return_code =1;
		REACCESS(Light)(&(map_drawing_information->light),light);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_light."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_light */

struct Light_model *get_map_drawing_information_light_model(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Light_model of the <map_drawing_information>.
==============================================================================*/
{
	struct Light_model *light_model;
	ENTER(get_map_drawing_information_light_model);
	if (map_drawing_information)
	{
		light_model=map_drawing_information->light_model;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_light_model."
				" invalid arguments");
		light_model = (struct Light_model *)NULL;
	}
	LEAVE;
	
	return (light_model);
} /* get_map_drawing_information_light_model */

int set_map_drawing_information_light_model(
	struct Map_drawing_information *map_drawing_information,
	struct Light_model *light_model)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Light_model of the <map_drawing_information>.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_light_model);
	if (map_drawing_information)
	{
		return_code =1;
		REACCESS(Light_model)(&(map_drawing_information->light_model),light_model);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_light_model."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_light_model */

struct Scene_viewer *get_map_drawing_information_scene_viewer(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Scene_viewer of the <map_drawing_information>.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;
	ENTER(get_map_drawing_information_scene_viewer);
	if (map_drawing_information)
	{
		scene_viewer=map_drawing_information->scene_viewer;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_scene_viewer."
				" invalid arguments");
		scene_viewer = (struct Scene_viewer *)NULL;
	}
	LEAVE;
	
	return (scene_viewer);
} /* get_map_drawing_information_scene_viewer */

int set_map_drawing_information_scene_viewer(
	struct Map_drawing_information *map_drawing_information,
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Scene_viewer of the <map_drawing_information>.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_scene_viewer);
	if (map_drawing_information)
	{
		return_code =1;
		map_drawing_information->scene_viewer=scene_viewer;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_scene_viewer."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_scene_viewer */

struct Scene *get_map_drawing_information_scene(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Scene of the <map_drawing_information>.
==============================================================================*/
{
	struct Scene *scene;
	ENTER(get_map_drawing_information_scene);
	if (map_drawing_information)
	{
		scene=map_drawing_information->scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_scene."
				" invalid arguments");
		scene = (struct Scene *)NULL;
	}
	LEAVE;
	
	return (scene);
} /* get_map_drawing_information_scene */

int set_map_drawing_information_scene(
	struct Map_drawing_information *map_drawing_information,
	struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Scene of the <map_drawing_information>.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_scene);
	if (map_drawing_information)
	{
		return_code =1;
		REACCESS(Scene)(&(map_drawing_information->scene),scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_scene."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_scene */

struct User_interface *get_map_drawing_information_user_interface(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the User_interface of the <map_drawing_information>.
==============================================================================*/
{
	struct User_interface *user_interface;
	ENTER(get_map_drawing_information_user_interface);
	if (map_drawing_information)
	{
		user_interface=map_drawing_information->user_interface;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_user_interface."
				" invalid arguments");
		user_interface = (struct User_interface *)NULL;
	}
	LEAVE;
	
	return (user_interface);
} /* get_map_drawing_information_user_interface */

int set_map_drawing_information_user_interface(
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the User_interface of the <map_drawing_information>.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_user_interface);
	if (map_drawing_information)
	{
		return_code =1;
		/* don't ACCESS as there's only ever one user_interface */
		map_drawing_information->user_interface=user_interface;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_user_interface."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_user_interface */

struct Graphical_material *get_map_drawing_information_map_graphical_material(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the map_graphical_material of the <map_drawing_information>.
==============================================================================*/
{
	struct Graphical_material *map_graphical_material;
	ENTER(get_map_drawing_information_map_graphical_material);
	if (map_drawing_information)
	{
		map_graphical_material=map_drawing_information->map_graphical_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_map_graphical_material."
				" invalid arguments");
		map_graphical_material = (struct Graphical_material *)NULL;
	}
	LEAVE;
	
	return (map_graphical_material);
} /* get_map_drawing_information_map_graphical_material */

int set_map_drawing_information_map_graphical_material(
	struct Map_drawing_information *map_drawing_information,
	struct Graphical_material *map_graphical_material)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the map_graphical_material of the <map_drawing_information>.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_map_graphical_material);
	if (map_drawing_information)
	{
		return_code =1;
		REACCESS(Graphical_material)
			(&(map_drawing_information->map_graphical_material),map_graphical_material);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_map_graphical_material."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_map_graphical_material */

struct Graphical_material *get_map_drawing_information_electrode_graphical_material(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July  2000

DESCRIPTION :
gets the electrode_graphical_material of the <map_drawing_information>.
==============================================================================*/
{
	struct Graphical_material *electrode_graphical_material;
	ENTER(get_map_drawing_information_electrode_graphical_material);
	if (map_drawing_information)
	{
		electrode_graphical_material=
			map_drawing_information->electrode_graphical_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_electrode_graphical_material."
				" invalid arguments");
		electrode_graphical_material = (struct Graphical_material *)NULL;
	}
	LEAVE;
	
	return (electrode_graphical_material);
} /* get_map_drawing_information_electrode_graphical_material */

struct Computed_field_package *get_map_drawing_information_computed_field_package(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July  2000

DESCRIPTION :
gets the Computed_field_package of the <map_drawing_information>.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;
	ENTER(get_map_drawing_information_computed_field_package);
	if (map_drawing_information)
	{
		computed_field_package=map_drawing_information->computed_field_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_computed_field_package."
				" invalid arguments");
		computed_field_package = (struct Computed_field_package *)NULL;
	}
	LEAVE;
	
	return (computed_field_package);
} /* get_map_drawing_information_computed_field_package */

int set_map_drawing_information_computed_field_package(
	struct Map_drawing_information *map_drawing_information,
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 14 July  2000

DESCRIPTION :
Sets the Computed_field_package of the <map_drawing_information>.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_computed_field_package);
	if (map_drawing_information)
	{
		return_code =1;
		/* don't ACCESS as only one*/
		map_drawing_information->computed_field_package=computed_field_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_computed_field_package."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_computed_field_package */

struct Time_keeper *get_map_drawing_information_time_keeper(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Time_keeper of the <map_drawing_information>.
==============================================================================*/
{
	struct Time_keeper *time_keeper;
	ENTER(get_map_drawing_information_time_keeper);
	if (map_drawing_information)
	{
		time_keeper=map_drawing_information->time_keeper;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_time_keeper."
				" invalid arguments");
		time_keeper = (struct Time_keeper *)NULL;
	}
	LEAVE;
	
	return (time_keeper);
} /* get_map_drawing_information_time_keeper */

int set_map_drawing_information_time_keeper(
	struct Map_drawing_information *map_drawing_information,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Time_keeper of the <map_drawing_information>.
==============================================================================*/
{
	int return_code;

	ENTER(set_map_drawing_information_time_keeper);
	if (map_drawing_information)
	{
		return_code =1;
		REACCESS(Time_keeper)(&(map_drawing_information->time_keeper),time_keeper);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_map_drawing_information_time_keeper."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
} /* set_map_drawing_information_time_keeper */

int map_drawing_information_make_map_scene(
	struct Map_drawing_information *map_drawing_information,
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Creates the map_drawing_information scene, if isn't already present.
==============================================================================*/
{
	int return_code;
	struct Scene *map_scene;

	ENTER(map_drawing_information_make_map_scene);
	map_scene=(struct Scene *)NULL;
	if (map_drawing_information&&package)
	{
		/* create the map scene, if haven't already got one */
		if (!(map_drawing_information->scene))
		{
			if (map_scene=CREATE(Scene)("map"))
			{
				Scene_enable_graphics(map_scene,map_drawing_information->glyph_list,
					map_drawing_information->graphical_material_manager,
					map_drawing_information->map_graphical_material,
					map_drawing_information->light_manager,
					map_drawing_information->spectrum_manager,
					map_drawing_information->spectrum,
					map_drawing_information->texture_manager);

				Scene_set_graphical_element_mode(map_scene,GRAPHICAL_ELEMENT_EMPTY,
					Computed_field_package_get_computed_field_manager(
					map_drawing_information->computed_field_package),
					package->element_manager,
					package->element_group_manager,
					package->fe_field_manager,
					package->node_manager,
					package->node_group_manager,
					map_drawing_information->data_manager,
					package->data_group_manager,
					map_drawing_information->element_point_ranges_selection,
					map_drawing_information->element_selection,
					map_drawing_information->node_selection,
					map_drawing_information->data_selection,
					map_drawing_information->user_interface);

				/*???RC.  May want to use functions to modify default_scene here */
				/* eg. to add model lights, etc. */
				/* ACCESS so can never be destroyed */
				/*???RC.  Should be able to change: eg. gfx set default scene NAME */
				if (ADD_OBJECT_TO_MANAGER(Scene)(map_scene,
					map_drawing_information->scene_manager))
				{
					Scene_enable_time_behaviour(map_scene,
						map_drawing_information->time_keeper);
					set_map_drawing_information_scene(map_drawing_information,map_scene);
				}
				else
				{
					display_message(ERROR_MESSAGE,"map_drawing_information_make_map_spectrum_and_scene."
						" couldn't add map to scene");
					return_code =0;
				}
			}

		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"map_drawing_information_make_map_spectrum_and_scene."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	
	return (return_code);
}/* map_drawing_information_make_map_scene */

struct MANAGER(Spectrum) *get_map_drawing_information_spectrum_manager(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets the spectrum_manager of the <map_drawing_information>.
==============================================================================*/
{
	struct MANAGER(Spectrum) *spectrum_manager;
	ENTER(get_map_drawing_information_spectrum_manager);
	if (map_drawing_information)
	{
		spectrum_manager=map_drawing_information->spectrum_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_spectrum_manager."
				" invalid arguments");
		spectrum_manager = (struct MANAGER(Spectrum) *)NULL;
	}
	LEAVE;
	
	return (spectrum_manager);
} /* get_map_drawing_information_ spectrum_manager*/

struct LIST(GT_object) *get_map_drawing_information_glyph_list(
	struct Map_drawing_information *map_drawing_information)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
gets the glyph_list of the <map_drawing_information>
==============================================================================*/
{
	struct LIST(GT_object) *glyph_list;
	ENTER(get_map_drawing_information_glyph_list);
	if (map_drawing_information)
	{
		glyph_list=map_drawing_information->glyph_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_map_drawing_information_glyph_list."
				" invalid arguments");
		glyph_list = (struct LIST(GT_object) *)NULL;
	}
	LEAVE;
	
	return (glyph_list);
} /* get_map_drawing_information_glyph_list */

int map_drawing_information_align_scene(
	struct Map_drawing_information *map_drawing_information, double up_vector[3]) /*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION : Aligns the <map_drawing_information> scene so that
if <up_vector> is not NULL, it is the scene viewer's up vector, else
the  largest cardinal dimension
component of the scene is the scene viewer's up vector, and the viewer is
looking along the scene's smallest cardinal dimension component towards the
centre of the scene. Eg if a scene is 100 by 30 by 150 (in x,y,z)
then we'd look along the y axis, with the z axis as the up vector.

cf Scene_viewer_view_all.
Should be in Scene_viewer? RC doesn't think so.
==============================================================================*/
{
	double centre[3],clip_factor,diff[3],dist,eye[3],lookat[3],max,min,
		radius,size[3],up[3];
	int i,max_index,min_index,return_code,width_factor;
	struct Scene_viewer *scene_viewer=(struct Scene_viewer *)NULL;
	/*not using middle at the moment, but may want to in future*/
#if defined (NEW_CODE)
	double middle;
	int middle_index
#endif /* defined (NEW_CODE)*/

	ENTER(map_drawing_information_align_scene);
	if (map_drawing_information&&(scene_viewer=
		get_map_drawing_information_scene_viewer(map_drawing_information)))
	{
		if (Scene_get_graphics_range(Scene_viewer_get_scene(scene_viewer),
			&centre[0],&centre[1],&centre[2],&size[0],&size[1],&size[2]))
		{
			radius=0.5*sqrt(size[0]*size[0] + size[1]*size[1] + size[2]*size[2]);
			/*set radius to 1, so don't get error with Scene_viewer_set_view_simple,*/
			/*just an empty scene*/
			if (radius<1)
			{
				radius=1;
			}
			/* enlarge radius to keep image within edge of window */
			/*???RC width_factor should be read in from defaults file */
			width_factor=1.05;
			radius *= width_factor;
			/*???RC clip_factor should be read in from defaults file: */
			clip_factor = 10.0;
			if ((Scene_viewer_set_view_simple(scene_viewer,centre[0],centre[1],
				centre[2],radius,40,clip_factor*radius))&&
			/*now done the equiv of Scene_viewer_view_all,&  have size[0],&size[1],&size[2] */
			(Scene_viewer_get_lookat_parameters(scene_viewer,
				&eye[0],&eye[1],&eye[2],&lookat[0],&lookat[1],&lookat[2],&up[0],&up[1],
				&up[2])))
			{
				/* get,min,middle,max component of the scene size*/
				min=size[0];
				max=size[0];
				min_index=0;
				max_index=0;
				for(i=0;i<3;i++)
				{
					if (size[i]>max)
					{
						max=size[i];
						max_index=i;
					}
					if (size[i]<min)
					{
						min=size[i];
						min_index=i;
					}
				}
				/*not using middle at the moment, but may want to in future*/
#if defined (NEW_CODE)
				middle=size[0];
				middle_index=0;
				for(i=0;i<3;i++)
				{
					if ((size[i]<max)&&(size[i]>min))
					{
						middle=size[i];
						middle_index=i;
					}
				}
#endif /*defined (NEW_CODE)	 */
				/* to prevent colinearity of eye[] and up[] vectors*/
				if (min_index==max_index)
				{
					if (min_index<2)
					{
						min_index++;
					}
					else
					{
						min_index--;
					}
				}
				/* if we've specified an up_vector, use it*/
				if (up_vector)
				{
					for(i=0;i<3;i++)
					{
						up[i]=up_vector[i];
					}
				}
				else
				/*else calculated one*/
				{
					/*clear up vector */
					for(i=0;i<3;i++)
					{
						up[i]=0;
					}
					/*set the up vector to be the max size component*/
					up[max_index]=1;
				}
				/* get the dist of eye pt from centre pt (after Scene_viewer_set_view_simple ) */
				/* (centre[] and lookat[] are the same)*/
				diff[0]=eye[0]-centre[0];
				diff[1]=eye[1]-centre[1];
				diff[2]=eye[2]-centre[2];
				dist=sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
				/*clear eye pt*/
				for(i=0;i<3;i++)
				{
					eye[i]=0;
				}
				/*set eye pt to be along the min size component,dist from the centre */
				/*-ve sign so look at the front of torso meshes - they seem to be aligned this way*/
				eye[min_index]=-dist;
				return_code=Scene_viewer_set_lookat_parameters(scene_viewer,
					eye[0],eye[1],eye[2],centre[0],centre[1],centre[2],up[0],up[1],up[2]);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"map_drawing_information_align_scene. Scene_viewer_set_view_simple failed");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"map_drawing_information_align_scene. could not get graphics range");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"map_drawing_information_align_scene.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /*map_drawing_information_align_scene  */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int define_fit_field_at_quad_elements_and_nodes(
	struct GROUP(FE_element) *torso_element_group,
	struct FE_field *fit_field,struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 7 December 2000

DESCRIPTION :
Finds all the elements in <torso_element_group> with 4 nodes,
for these elements defines <fit_field> at the element, and it's nodes.
===============================================================================*/
{
	int return_code;
	struct Define_fit_field_at_torso_element_nodes_data
		define_fit_field_at_torso_element_nodes_data;

	ENTER(define_fit_field_at_quad_elements_and_nodes);
	if (torso_element_group&&fit_field)
	{
		define_fit_field_at_torso_element_nodes_data.quad_element_group=
			CREATE(GROUP(FE_element))("quad");
		define_fit_field_at_torso_element_nodes_data.fit_field=fit_field;
		define_fit_field_at_torso_element_nodes_data.node_manager=node_manager;
		/* set up the quad element group, and add the fit field to the nodes*/
		FOR_EACH_OBJECT_IN_GROUP(FE_element)(define_fit_field_at_torso_element_nodes,
			(void *)(&define_fit_field_at_torso_element_nodes_data),torso_element_group);
		/* put the fit field on the quad group elements */
		return_code=set_up_fitted_potential_on_torso(
			define_fit_field_at_torso_element_nodes_data.quad_element_group,
			fit_field,basis_manager,element_manager);
		/* have altered the elements, but no longer need the group */
		DESTROY(GROUP(FE_element))
			(&define_fit_field_at_torso_element_nodes_data.quad_element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"define_fit_field_at_quad_elements_and_nodes."
			" Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}/* define_fit_field_at_quad_elements_and_nodes */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
struct FE_field *create_mapping_type_fe_field(char *field_name,
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time)
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
creates a 1 component  <field_name>
==============================================================================*/
{
	struct FE_field *map_fit_field;
	struct Coordinate_system coordinate_system;

	ENTER(create_mapping_type_fe_field);
	map_fit_field=(struct FE_field *)NULL;
	if (field_name&&fe_field_manager)
	{
		/* set up the info needed to create the potential field */
		coordinate_system.type=NOT_APPLICABLE;
		/* create the potential  field, add it to the node */
		if (!(map_fit_field=get_FE_field_manager_matched_field(fe_field_manager,
			field_name,GENERAL_FE_FIELD,fe_time,/*indexer_field*/(struct FE_field *)NULL,
			/*number_of_indexed_values*/0,CM_GENERAL_FIELD,
			&coordinate_system,FE_VALUE_VALUE,
			/*number_of_components*/1,fit_comp_name,
			/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
			(struct FE_field_external_information *)NULL)))
		{
			display_message(ERROR_MESSAGE,"create_mapping_type_fe_field.  "
				"Could not retrieve potential_value field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"create_mapping_type_fe_field. invalid argument");
	}
	LEAVE;

	return (map_fit_field);
}/* create_mapping_type_fe_field */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
struct Torso_node_data
{
	struct Computed_field *cylindrical_polar_coords_wrapper,
		*transformed_rect_cart_coords,*rect_cart_coords_wrapper;
	struct FE_field *cylindrical_polar_coords,*rect_cart_coords;
	struct MANAGER(FE_node) *node_manager;
	struct FE_time_version *time_version;
	struct LIST(Computed_field) *computed_field_list;
}; /* Torso_node_data */

int iterative_add_cylindrical_info_to_cartesian_torso_node(
	struct FE_node *torso_node,void *torso_node_data_void)
/*******************************************************************************
LAST MODIFIED : 5 February 2002

DESCRIPTION :
Add cylindrical polar information to a node of the the default torso,
so can do a seamless texture map.
Called by
===============================================================================*/
{
	char **computed_field_names,**current_computed_field_name,*type_str,
		end_string[10],version_num_string[20];
	enum FE_nodal_value_type *nodal_derivative_types,nodal_value_type;
	int i,j,*max_versions,number_of_computed_fields,number_of_derivative_versions,
		number_of_derivatives,number_of_name_allocations,return_code,string_error,
		version_display_number;
	struct Computed_field *cylindrical_polar_coords_wrapper,
		*transformed_rect_cart_coords,*rect_cart_coords_wrapper,*source,*dest,
		*trans_source;
	struct Coordinate_system coordinate_system;
	struct FE_field *cylindrical_polar_coords,*rect_cart_coords;
	struct FE_node *node_copy;
	struct FE_node_field_creator *node_field_creator;
	struct FE_time_version *time_version;
	struct MANAGER(FE_node) *node_manager;
	struct Torso_node_data  *torso_node_data;
	struct LIST(Computed_field) *computed_field_list;

	ENTER(iterative_add_cylindrical_info_to_cartesian_torso_node);
	source=(struct Computed_field *)NULL;
	dest=(struct Computed_field *)NULL;
	trans_source=(struct Computed_field *)NULL;
	max_versions=(int *)NULL;
	computed_field_names=(char ** )NULL;
	current_computed_field_name=(char ** )NULL;
	nodal_derivative_types=(enum FE_nodal_value_type *)NULL;
	cylindrical_polar_coords=(struct FE_field *)NULL;
	node_copy=(struct FE_node *)NULL;
	torso_node_data=(struct Torso_node_data  *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	time_version = (struct FE_time_version *)NULL;
	computed_field_list=(struct LIST(Computed_field) *)NULL;
	if (torso_node&&torso_node_data_void&&(torso_node_data=
		(struct Torso_node_data *)torso_node_data_void))
	{
		coordinate_system.type=CYLINDRICAL_POLAR;
		computed_field_list=torso_node_data->computed_field_list;
		rect_cart_coords=torso_node_data->rect_cart_coords;
		rect_cart_coords_wrapper=torso_node_data->rect_cart_coords_wrapper;
		transformed_rect_cart_coords=torso_node_data->transformed_rect_cart_coords;
		cylindrical_polar_coords=torso_node_data->cylindrical_polar_coords;
		cylindrical_polar_coords_wrapper=torso_node_data->cylindrical_polar_coords_wrapper;
		node_manager=torso_node_data->node_manager;
		time_version=torso_node_data->time_version;
		if (FE_field_is_defined_at_node(rect_cart_coords,torso_node))
		{
			/* create a node to work with */
			if (node_copy = CREATE(FE_node)(0, torso_node))
			{
				/*define the cylindrical_polar_coords at the node*/
				if (node_field_creator = create_FE_node_field_creator_from_node_field(
					node_copy, rect_cart_coords))
				{
					if (define_FE_field_at_node(node_copy, cylindrical_polar_coords,
						time_version, node_field_creator))
					{
						if (Computed_field_copy_values_at_node(node_copy,
							cylindrical_polar_coords_wrapper,
							transformed_rect_cart_coords, /*time*/0.0))
						{
							/* determine the number of derivative_types, and number of versions of each*/
							if (return_code =FE_node_field_creator_get_nodal_derivative_versions(
								node_field_creator,&number_of_derivatives,&nodal_derivative_types,
									&max_versions))
							{
								number_of_derivative_versions=0;
								for(i=0;i<number_of_derivatives;i++)
								{
									for(j=0;j<max_versions[i];j++)
									{
										number_of_derivative_versions++;
									}
								}
								number_of_computed_fields=number_of_derivative_versions*3;
								/*allocate space for pointers to computed field names*/
								if (ALLOCATE(computed_field_names,char *,number_of_computed_fields))
								{
									number_of_name_allocations=0;
									current_computed_field_name=computed_field_names;
									/*allocate space computed field names */
									for(i=0;(i<number_of_computed_fields)&&return_code;i++)
									{
										if (ALLOCATE(*current_computed_field_name,char,30))
										{
											current_computed_field_name++;
											number_of_name_allocations++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"iterative_add_cylindrical_info_to_cartesian_torso_node.  "
												"Could not allocate current_computed_field_names");
											return_code = 0;
										}
									}
									/*construct the names*/
									if (return_code)
									{
										current_computed_field_name=computed_field_names;
										for(i=0;(i<number_of_derivatives)&&return_code;i++)
										{
											for(j=0;(j<max_versions[i])&&return_code;j++)
											{
												version_display_number=j+1;
												nodal_value_type=nodal_derivative_types[i];
												type_str=ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type);
												strcpy(end_string,type_str);
												sprintf(version_num_string,"_v%d",version_display_number);
												strcat(end_string,version_num_string);
												strcpy(*current_computed_field_name,"src_");
												append_string(current_computed_field_name,end_string,&string_error);
												if (!(source=FIND_BY_IDENTIFIER_IN_LIST(Computed_field, name)
													(*current_computed_field_name,computed_field_list)))
												{
													/*create computed field and add to list*/
													if (source=CREATE(Computed_field)(*current_computed_field_name))
													{
														if (!((Computed_field_set_type_node_value(source,
															rect_cart_coords,nodal_value_type,/* version_number*/j))&&
															(ADD_OBJECT_TO_LIST(Computed_field)(source,computed_field_list))))
														{
															return_code=0;
															DESTROY(Computed_field)(&source);
														}
													}
													else
													{
														return_code=0;
													}
												}
												current_computed_field_name++;

												if (return_code)
												{
													strcpy(*current_computed_field_name,"trans_src_");
													append_string(current_computed_field_name,end_string,&string_error);
													if (!(trans_source=FIND_BY_IDENTIFIER_IN_LIST(Computed_field, name)
														(*current_computed_field_name,computed_field_list)))
													{
														/*create computed field and add to list*/
														if (trans_source=CREATE(Computed_field)(*current_computed_field_name))
														{
															if (!((Computed_field_set_type_vector_coordinate_transformation(
																trans_source,source,rect_cart_coords_wrapper))&&
																(Computed_field_set_coordinate_system(trans_source,
																	&coordinate_system))&&
																(ADD_OBJECT_TO_LIST(Computed_field)(trans_source,
																	computed_field_list))))
															{
																return_code=0;
																DESTROY(Computed_field)(&source);
															}
														}
														else
														{
															return_code=0;
														}
													}
												}
												current_computed_field_name++;

												if (return_code)
												{
													strcpy(*current_computed_field_name,"dest_");
													append_string(current_computed_field_name,end_string,&string_error);
													if (!(dest=FIND_BY_IDENTIFIER_IN_LIST(Computed_field, name)
														(*current_computed_field_name,computed_field_list)))
													{
														/*create computed field and add to list*/
														if (dest=CREATE(Computed_field)(*current_computed_field_name))
														{
															if (!((Computed_field_set_type_node_value(dest,
																cylindrical_polar_coords,nodal_value_type,/* version_number*/j))&&
																(ADD_OBJECT_TO_LIST(Computed_field)(dest,computed_field_list))))
															{
																return_code=0;
																DESTROY(Computed_field)(&source);
															}
														}
														else
														{
															return_code=0;
														}
													}
													if (return_code)
													{
														/* copy the nodal derivatives  */
														Computed_field_copy_values_at_node(node_copy,
															dest, trans_source, 0/*time is zero*/);
													}
												}/* if (return_code)*/
												current_computed_field_name++;
											}/* for(j=0;(j<max_versions[i])&&return_code;j++) */
										} /* for(i=0;(i<number_of_derivatives)&&return_code;i++)*/
									}/* if (return_code) */

									/*deallocate names*/
									current_computed_field_name=computed_field_names;
									for(i=0;i<number_of_name_allocations;i++)
									{
										DEALLOCATE(*current_computed_field_name);
										current_computed_field_name++;
									}
									DEALLOCATE(computed_field_names);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"iterative_add_cylindrical_info_to_cartesian_torso_node.  "
										"Could not allocate computed_field_names");
									return_code = 0;
								}
							}
							DEALLOCATE(nodal_derivative_types);
							DEALLOCATE(max_versions);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"iterative_add_cylindrical_info_to_cartesian_torso_node.  "
								"Could not transform coordinates");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"iterative_add_cylindrical_info_to_cartesian_torso_node.  "
							"Could not define field at node");
						return_code = 0;
					}
					DESTROY(FE_node_field_creator)(&node_field_creator);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"iterative_add_cylindrical_info_to_cartesian_torso_node.  "
						"Could not create node field creator");
					return_code = 0;
				}
				/* copy it back into the manager */
				MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)
					(torso_node,node_copy,node_manager);
				DESTROY(FE_node)(&node_copy);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"iterative_add_cylindrical_info_to_cartesian_torso_node.  Could not create node copy");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"iterative_add_cylindrical_info_to_cartesian_torso_node.  Source coordinates not defined");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"iterative_add_cylindrical_info_to_cartesian_torso_node."
			" Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* iterative_add_cylindrical_info_to_cartesian_torso_node*/

#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int add_cylindrical_info_to_cartesian_torso(char *torso_name,
	struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 5 February 2002

DESCRIPTION :
Add cylindrical polar information to the default torso, so can do a seamless
texture map.
==============================================================================*/
{
	char *cylindrical_polar_component_names[3]=
	{
		"r","theta","z"
	};
	int return_code;
	struct Coordinate_system coordinate_system;
	struct Computed_field *torso_position_field_wrapper,*transformed_rect_cart_coords;
	struct FE_field *cylindrical_polar_coords,*torso_position_field;
	struct FE_node *torso_node;
	struct FE_time *fe_time;
	struct GROUP(FE_node) *torso_node_group;
	struct GROUP(FE_element) *torso_element_group;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct Torso_node_data  torso_node_data;

	ENTER(add_cylindrical_info_to_cartesian_torso);
	torso_element_group=(struct GROUP(FE_element) *)NULL;
	torso_node=(struct FE_node *)NULL;
	node_manager=(struct MANAGER(FE_node) *)NULL;
	element_manager=(struct MANAGER(FE_element) *)NULL;
	cylindrical_polar_coords=(struct FE_field *)NULL;
	fe_time=(struct FE_time *)NULL;
	torso_node_group=(struct GROUP(FE_node)  *)NULL;
	fe_field_manager=(struct  MANAGER(FE_field) *)NULL;
	node_group_manager=(struct MANAGER(GROUP(FE_node)) *)NULL;
	element_group_manager=(struct MANAGER(GROUP(FE_element)) *)NULL;
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	torso_position_field_wrapper=(struct Computed_field *)NULL;
	transformed_rect_cart_coords=(struct Computed_field *)NULL;
	if (torso_name&&
		(fe_field_manager=get_unemap_package_FE_field_manager(unemap_package))
		&&(node_manager=get_unemap_package_node_manager(unemap_package))
		&&(element_manager=get_unemap_package_element_manager(unemap_package))
		&&(node_group_manager=get_unemap_package_node_group_manager(unemap_package))
		&&(element_group_manager=get_unemap_package_element_group_manager(unemap_package))
		&&(computed_field_manager=get_unemap_package_Computed_field_manager(unemap_package))
		&&(torso_node_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)
			(torso_name,node_group_manager))
		&&(torso_element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
			(torso_name,element_group_manager)))
	{
		return_code=1;
		fe_time=get_unemap_package_FE_time(unemap_package);
		coordinate_system.type=CYLINDRICAL_POLAR;
		/* create the cylindrical_polar_coords field, */
		if (!(cylindrical_polar_coords=get_FE_field_manager_matched_field(
			fe_field_manager,"um_cylindrical_polar_coords",
			GENERAL_FE_FIELD,fe_time,/*indexer_field*/(struct FE_field *)NULL,
			/*number_of_indexed_values*/0,CM_COORDINATE_FIELD,
			&coordinate_system,FE_VALUE_VALUE,
			/*number_of_components*/3,cylindrical_polar_component_names,
			/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE,
			(struct FE_field_external_information *)NULL)))
		{
			display_message(ERROR_MESSAGE,
				"add_cylindrical_info_to_cartesian_torso. Could not retrieve um_cylindrical_polar_coords");
			return_code=0;
		}
		if ((torso_node=FIRST_OBJECT_IN_GROUP_THAT(FE_node)
			((GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL,torso_node_group))&&
		(torso_position_field=get_FE_node_default_coordinate_field(torso_node)))
		{
			if (torso_position_field_wrapper=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)
				(Computed_field_is_read_only_with_fe_field,(void *)
					(torso_position_field),computed_field_manager))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"add_cylindrical_info_to_cartesian_torso."
					" Couldn't retrieve torso_position_field_wrapper");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"add_cylindrical_info_to_cartesian_torso. erro finding torso_node/field");
			return_code=0;
		}
		if (return_code)
		{
			coordinate_system.type=CYLINDRICAL_POLAR;
			if (!((transformed_rect_cart_coords=CREATE(Computed_field)
				("um_transformed_rect_cart_coords")) &&
				Computed_field_set_coordinate_system(transformed_rect_cart_coords,
					&coordinate_system) &&
				Computed_field_set_type_coordinate_transformation(transformed_rect_cart_coords,
					torso_position_field_wrapper)))
			{
				DESTROY(Computed_field)(&transformed_rect_cart_coords);
			}
			torso_node_data.rect_cart_coords=torso_position_field;
			torso_node_data.rect_cart_coords_wrapper=torso_position_field_wrapper;
			torso_node_data.cylindrical_polar_coords=cylindrical_polar_coords;
			torso_node_data.node_manager=node_manager;
			torso_node_data.time_version=
				(struct FE_time_version *)NULL;
			torso_node_data.cylindrical_polar_coords_wrapper=
				FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)
				(Computed_field_is_read_only_with_fe_field,(void *)
					(cylindrical_polar_coords),computed_field_manager);
			torso_node_data.transformed_rect_cart_coords=transformed_rect_cart_coords;
			torso_node_data.computed_field_list=CREATE(LIST(Computed_field))();
			if (torso_node_data.transformed_rect_cart_coords &&
				torso_node_data.cylindrical_polar_coords_wrapper &&
				torso_node_data.computed_field_list)
			{
				/*define the cylindrical_polar_coords field at all the torso nodes*/
				MANAGER_BEGIN_CACHE(FE_node)(node_manager);
				return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(
					iterative_add_cylindrical_info_to_cartesian_torso_node,
					(void *)(&torso_node_data),torso_node_group);
				MANAGER_END_CACHE(FE_node)(node_manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"add_cylindrical_info_to_cartesian_torso.  Failed");
				return_code = 0;
			}
			/*destroy stuff in torso_node_data*/
			DESTROY(Computed_field)(&transformed_rect_cart_coords);
			DESTROY(LIST(Computed_field))(&(torso_node_data.computed_field_list));
			if (return_code)
			{
				/*add the cylindrical_polar_coords field to the elements*/
				struct Dfn_FE_field_at_elem_frm_template_data
					dfn_FE_field_at_elem_frm_template_data;

				dfn_FE_field_at_elem_frm_template_data.field=cylindrical_polar_coords;
				dfn_FE_field_at_elem_frm_template_data.template_field=torso_position_field;
				dfn_FE_field_at_elem_frm_template_data.element_manager=element_manager;
				MANAGER_BEGIN_CACHE(FE_element)(element_manager);
				FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					iterative_define_FE_field_cp_at_element_from_template_field_modify_theta,
					(void *)&dfn_FE_field_at_elem_frm_template_data,torso_element_group);
				MANAGER_END_CACHE(FE_element)(element_manager);
			}/* if (return_code)*/
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
				"add_cylindrical_info_to_cartesian_torso. invalid argument");
	}
	LEAVE;
	
	return (return_code);
}/*add_cylindrical_info_to_cartesian_torso*/
#endif /* defined (UNEMAP_USE_3D) */

