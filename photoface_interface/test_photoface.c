/*******************************************************************************
FILE : test_photoface.c

LAST MODIFIED : 31 January 2001

DESCRIPTION :
Tests the photoface interface.
==============================================================================*/

#include <stdio.h>
#include "photoface_cmiss.h"
#include "photoface_cmiss_utilities.h"

struct Obj
{ 
  int number_of_vertices;
  float *vertex_3d_locations;
  int number_of_texture_vertices;
  float *texture_vertex_3d_locations;
  int number_of_triangles;
  int *triangle_vertices;
  int *triangle_texture_vertices;
};

#define IMAGE_WIDTH (946)
#define IMAGE_HEIGHT (1282)
#define TEXTURE_WIDTH (512)
#define TEXTURE_HEIGHT (512)

int main(int argc, char **argv)
/*******************************************************************************
LAST MODIFIED : 24 January 2001

DESCRIPTION :
Runs a job through the photoface interface.
==============================================================================*/
{
  /* input supplied to pf_specify_markers */
  int number_of_markers = 175;
  char *marker_names[] = {
    "Skull_Top_Right_2",
    "Skull_Top_Right_1",
    "Skull_Top_Left_End",
    "Skull_Top_Right_End",
    "Skull_Top_Left_2",
    "Skull_Top_Left_1",
    "Skull_Top_Vertex",
    "Hairline_Right_End",
    "Hairline_Right_3",
    "Hairline_Right_1",
    "Hairline_Trichion",
    "Hairline_Left_1",
    "Hairline_Left_3",
    "Hairline_Left_End",
    "Hairline_Left_2",
    "Hairline_Right_2",
    "Ear_Left_Vertex",
    "Ear_Left_Obs_Vertex",
    "Ear_Left_Obs",
    "Ear_Left_Obi",
    "Ear_Left_Lobe",
    "Ear_Left_Out_1",
    "Ear_Left_Out_4",
    "Ear_Left_Out_2",
    "Ear_Left_Out_3",
    "Ear_Left_Out_5",
    "Ear_Right_Out_4",
    "Ear_Right_Lobe",
    "Ear_Right_Obs_Vertex",
    "Ear_Right_Vertex",
    "Ear_Right_Out_2",
    "Ear_Right_Obi",
    "Ear_Right_Obs",
    "Ear_Right_Out_1",
    "Ear_Right_Out_3",
    "Ear_Right_Out_5",
    "Jaw_Gnathon",
    "Jaw_Right_1",
    "Jaw_Left_1",
    "Jaw_Left_2",
    "Jaw_Left_3",
    "Jaw_Left_4",
    "Jaw_Right_2",
    "Jaw_Right_3",
    "Jaw_Right_4",
    "Jaw_Left_Obi",
    "Jaw_Right_Obi",
    "Jaw_Left_5",
    "Jaw_Right_5",
    "Eyebrow_Left_Upper_3",
    "Eyebrow_Left_Upper_2",
    "Eyebrow_Left_Upper_1",
    "Eyebrow_Left_Lower_1",
    "Eyebrow_Left_Lower_2",
    "Eyebrow_Left_Lower_3",
    "Eyebrow_Left_Outer_Corner",
    "Eyebrow_Left_Inner_Corner",
    "Eyebrow_Right_Upper_1",
    "Eyebrow_Right_Upper_2",
    "Eyebrow_Right_Upper_3",
    "Eyebrow_Right_Lower_1",
    "Eyebrow_Right_Lower_2",
    "Eyebrow_Right_Lower_3",
    "Eyebrow_Right_Inner_Corner",
    "Eyebrow_Right_Outer_Corner",
    "Eye_Left_Inner_Iris_0",
    "Eye_Left_Center_0",
    "Eye_Left_Exocanthion",
    "Eye_Left_Endocanthion",
    "Eye_Left_Top_Lid_1",
    "Eye_Left_Top_Lid_5",
    "Eye_Left_Bottom_Lid_5",
    "Eye_Left_Bottom_Lid_3",
    "Eye_Left_Bottom_Lid_1",
    "Eye_Left_Top_Lid_3",
    "Eye_Left_Outer_Iris_0",
    "Eye_Left_Pupil_Center_0",
    "Eye_Left_Top_Lid_4",
    "Eye_Left_Top_Lid_2",
    "Eye_Left_Bottom_Lid_2",
    "Eye_Left_Bottom_Lid_4",
    "Eye_Left_Top_Fold_2",
    "Eye_Left_Top_Fold_3",
    "Eye_Left_Top_Fold_4",
    "Eye_Left_Top_Fold_5",
    "Eye_Left_Top_Fold_6",
    "Eye_Left_Top_Fold_7",
    "Eye_Left_Top_Fold_Inner_Corner",
    "Eye_Left_Top_Fold_1",
    "Eye_Left_Top_Fold_Outer_Corner",
    "Eye_Left_Top_Lid_6",
    "Eye_Left_Top_Lid_7",
    "Eye_Left_Bottom_Lid_6",
    "Eye_Left_Bottom_Lid_7",
    "Eye_Right_Exocanthion",
    "Eye_Right_Endocanthion",
    "Eye_Right_Bottom_Lid_5",
    "Eye_Right_Bottom_Lid_3",
    "Eye_Right_Bottom_Lid_1",
    "Eye_Right_Top_Lid_1",
    "Eye_Right_Inner_Iris_0",
    "Eye_Right_Top_Lid_5",
    "Eye_Right_Top_Lid_3",
    "Eye_Right_Outer_Iris_0",
    "Eye_Right_Pupil_Center_0",
    "Eye_Right_Center_0",
    "Eye_Right_Top_Lid_2",
    "Eye_Right_Top_Lid_4",
    "Eye_Right_Bottom_Lid_2",
    "Eye_Right_Bottom_Lid_4",
    "Eye_Right_Top_Fold_6",
    "Eye_Right_Top_Fold_5",
    "Eye_Right_Top_Fold_4",
    "Eye_Right_Top_Fold_3",
    "Eye_Right_Top_Fold_2",
    "Eye_Right_Top_Fold_7",
    "Eye_Right_Top_Fold_Inner_Corner",
    "Eye_Right_Top_Fold_1",
    "Eye_Right_Top_Fold_Outer_Corner",
    "Eye_Right_Top_Lid_6",
    "Eye_Right_Top_Lid_7",
    "Eye_Right_Bottom_Lid_6",
    "Eye_Right_Bottom_Lid_7",
    "Nose_Midpoint",
    "Nose_Right_Top",
    "Nose_Right_2",
    "Nose_Right_1",
    "Nose_Right_3",
    "Nose_Right_Wing",
    "Nose_Right_Nostril_1",
    "Nose_Right_Nostril_2",
    "Nose_Right_Nostril_3",
    "Nose_Left_Top",
    "Nose_Left_Wing",
    "Nose_Left_3",
    "Nose_Left_2",
    "Nose_Left_1",
    "Nose_Nason_0",
    "Nose_Tip_0",
    "Nose_Left_Nostril_1",
    "Nose_Left_Nostril_2",
    "Nose_Left_Nostril_3",
    "Nose_Subnasale_0",
    "Lips_Upper_Left_Corner",
    "Lips_Upper_Outer_1",
    "Lips_Upper_Outer_2",
    "Lips_Upper_Right_Corner",
    "Lips_Upper_Outer_5",
    "Lips_Upper_Outer_4",
    "Lips_Upper_Inner_3",
    "Lips_Upper_Outer_3",
    "Lips_Upper_Inner_1",
    "Lips_Upper_Inner_2",
    "Lips_Upper_Inner_5",
    "Lips_Upper_Inner_4",
    "Lips_Lower_Right_Corner",
    "Lips_Lower_Outer_2",
    "Lips_Lower_Outer_3",
    "Lips_Lower_Outer_4",
    "Lips_Lower_Outer_5",
    "Lips_Lower_Inner_2",
    "Lips_Lower_Inner_4",
    "Lips_Lower_Inner_5",
    "Lips_Lower_Inner_1",
    "Lips_Lower_Left_Corner",
    "Lips_Lower_Inner_3",
    "Lips_Lower_Outer_1",
    "Neck_Left_Out_2",
    "Neck_Left_Top",
    "Neck_Left_Out_1",
    "Neck_Left_Bottom",
    "Neck_Right_Top",
    "Neck_Right_Bottom",
    "Neck_Right_Out_2",
    "Neck_Right_Out_1"};
  float marker_2d_positions[] = {
    642.331, 142.579, /* Skull_Top_Right_2 */
    562.164, 110.674, /* Skull_Top_Right_1 */
    235.524, 205.328, /* Skull_Top_Left_End */
    721.845, 214.937, /* Skull_Top_Right_End */
    317.859, 142.963, /* Skull_Top_Left_2 */
    400.346, 103.609, /* Skull_Top_Left_1 */
    481.363, 94.9067, /* Skull_Top_Vertex */
    718.376, 382.008, /* Hairline_Right_End */
    658.123, 285.546, /* Hairline_Right_3 */
    533.007, 225.786, /* Hairline_Right_1 */
    469.861, 218.157, /* Hairline_Trichion */
    406.018, 221.704, /* Hairline_Left_1 */
    277.752, 269.708, /* Hairline_Left_3 */
    211.902, 357.136, /* Hairline_Left_End */
    342.247, 236.304, /* Hairline_Left_2 */
    595.944, 247.062, /* Hairline_Right_2 */
    131.042, 534.097, /* Ear_Left_Vertex */
    170.982, 524.432, /* Ear_Left_Obs_Vertex */
    184.761, 571.295, /* Ear_Left_Obs */
    189.445, 758.211, /* Ear_Left_Obi */
    153.06, 747.042, /* Ear_Left_Lobe */
    113.642, 572.012, /* Ear_Left_Out_1 */
    134.363, 677.456, /* Ear_Left_Out_4 */
    118.758, 607.388, /* Ear_Left_Out_2 */
    127.108, 642.396, /* Ear_Left_Out_3 */
    138.026, 712.875, /* Ear_Left_Out_5 */
    793.745, 704.584, /* Ear_Right_Out_4 */
    750.21, 780.07, /* Ear_Right_Lobe */
    771.222, 550.11, /* Ear_Right_Obs_Vertex */
    808.39, 551.169, /* Ear_Right_Vertex */
    818.541, 626.241, /* Ear_Right_Out_2 */
    729.202, 772.205, /* Ear_Right_Obi */
    751.486, 593.059, /* Ear_Right_Obs */
    825.377, 591.969, /* Ear_Right_Out_1 */
    804.756, 665.027, /* Ear_Right_Out_3 */
    781.8, 751.947, /* Ear_Right_Out_5 */
    487.641, 1033.75, /* Jaw_Gnathon */
    571.506, 1015.65, /* Jaw_Right_1 */
    408.946, 1032.97, /* Jaw_Left_1 */
    338.953, 1017.32, /* Jaw_Left_2 */
    279.42, 973.003, /* Jaw_Left_3 */
    219.552, 895.544, /* Jaw_Left_4 */
    612.38, 992.712, /* Jaw_Right_2 */
    660.022, 958.408, /* Jaw_Right_3 */
    705.497, 880.466, /* Jaw_Right_4 */
    189.192, 759.248, /* Jaw_Left_Obi */
    728.599, 770.699, /* Jaw_Right_Obi */
    204.013, 831.297, /* Jaw_Left_5 */
    716.111, 827.026, /* Jaw_Right_5 */
    357.531, 464.669, /* Eyebrow_Left_Upper_3 */
    302.964, 463.051, /* Eyebrow_Left_Upper_2 */
    247.905, 480.861, /* Eyebrow_Left_Upper_1 */
    257.135, 499.134, /* Eyebrow_Left_Lower_1 */
    305.434, 491.906, /* Eyebrow_Left_Lower_2 */
    356.035, 491.806, /* Eyebrow_Left_Lower_3 */
    214.225, 519.589, /* Eyebrow_Left_Outer_Corner */
    400.438, 492.009, /* Eyebrow_Left_Inner_Corner */
    659.444, 479.019, /* Eyebrow_Right_Upper_1 */
    614.13, 465.861, /* Eyebrow_Right_Upper_2 */
    568.169, 463.502, /* Eyebrow_Right_Upper_3 */
    652.498, 498.308, /* Eyebrow_Right_Lower_1 */
    611.259, 493.71, /* Eyebrow_Right_Lower_2 */
    569.024, 493.066, /* Eyebrow_Right_Lower_3 */
    525.628, 487.676, /* Eyebrow_Right_Inner_Corner */
    686.959, 510.049, /* Eyebrow_Right_Outer_Corner */
    346.255, 557.109, /* Eye_Left_Inner_Iris_0 */
    324.34, 561.384, /* Eye_Left_Center_0 */
    281.659, 565.152, /* Eye_Left_Exocanthion */
    367.022, 563.616, /* Eye_Left_Endocanthion */
    290.178, 559.278, /* Eye_Left_Top_Lid_1 */
    335.766, 549.317, /* Eye_Left_Top_Lid_5 */
    335.525, 574.034, /* Eye_Left_Bottom_Lid_5 */
    310.45, 574.723, /* Eye_Left_Bottom_Lid_3 */
    289.615, 569.11, /* Eye_Left_Bottom_Lid_1 */
    310.411, 549.897, /* Eye_Left_Top_Lid_3 */
    296.752, 559.673, /* Eye_Left_Outer_Iris_0 */
    321.503, 558.391, /* Eye_Left_Pupil_Center_0 */
    322.126, 546.39, /* Eye_Left_Top_Lid_4 */
    298.697, 553.404, /* Eye_Left_Top_Lid_2 */
    297.572, 573.068, /* Eye_Left_Bottom_Lid_2 */
    323.328, 576.378, /* Eye_Left_Bottom_Lid_4 */
    281.277, 537.394, /* Eye_Left_Top_Fold_2 */
    298.293, 533.043, /* Eye_Left_Top_Fold_3 */
    318.088, 530.792, /* Eye_Left_Top_Fold_4 */
    338.38, 533.601, /* Eye_Left_Top_Fold_5 */
    360.017, 538.685, /* Eye_Left_Top_Fold_6 */
    371.899, 545.528, /* Eye_Left_Top_Fold_7 */
    379.19, 553.43, /* Eye_Left_Top_Fold_Inner_Corner */
    269.608, 542.974, /* Eye_Left_Top_Fold_1 */
    259.206, 548.213, /* Eye_Left_Top_Fold_Outer_Corner */
    349.407, 552.243, /* Eye_Left_Top_Lid_6 */
    358.214, 557.93, /* Eye_Left_Top_Lid_7 */
    347.722, 571.689, /* Eye_Left_Bottom_Lid_6 */
    357.372, 567.653, /* Eye_Left_Bottom_Lid_7 */
    636.725, 574.143, /* Eye_Right_Exocanthion */
    549.977, 566.478, /* Eye_Right_Endocanthion */
    578.695, 577.76, /* Eye_Right_Bottom_Lid_5 */
    605.092, 580.756, /* Eye_Right_Bottom_Lid_3 */
    627.586, 577.307, /* Eye_Right_Bottom_Lid_1 */
    628.743, 567.818, /* Eye_Right_Top_Lid_1 */
    569.546, 562.672, /* Eye_Right_Inner_Iris_0 */
    582.005, 553.896, /* Eye_Right_Top_Lid_5 */
    608.29, 556.863, /* Eye_Right_Top_Lid_3 */
    619.595, 566.268, /* Eye_Right_Outer_Iris_0 */
    594.571, 564.47, /* Eye_Right_Pupil_Center_0 */
    593.351, 566.637, /* Eye_Right_Center_0 */
    620.76, 561.492, /* Eye_Right_Top_Lid_2 */
    595.82, 552.233, /* Eye_Right_Top_Lid_4 */
    618.446, 580.471, /* Eye_Right_Bottom_Lid_2 */
    591.737, 581.04, /* Eye_Right_Bottom_Lid_4 */
    556.748, 549.243, /* Eye_Right_Top_Fold_6 */
    570.723, 543.717, /* Eye_Right_Top_Fold_5 */
    587.575, 540.293, /* Eye_Right_Top_Fold_4 */
    610.924, 541.375, /* Eye_Right_Top_Fold_3 */
    635.084, 547.309, /* Eye_Right_Top_Fold_2 */
    543.051, 557.358, /* Eye_Right_Top_Fold_7 */
    533.093, 567.042, /* Eye_Right_Top_Fold_Inner_Corner */
    649.309, 557.526, /* Eye_Right_Top_Fold_1 */
    663.337, 569.511, /* Eye_Right_Top_Fold_Outer_Corner */
    568.19, 555.559, /* Eye_Right_Top_Lid_6 */
    559.084, 561.018, /* Eye_Right_Top_Lid_7 */
    565.653, 574.479, /* Eye_Right_Bottom_Lid_6 */
    557.815, 570.478, /* Eye_Right_Bottom_Lid_7 */
    460.452, 770.4, /* Nose_Midpoint */
    492.599, 566.934, /* Nose_Right_Top */
    504.486, 658.417, /* Nose_Right_2 */
    496.121, 615.257, /* Nose_Right_1 */
    518.426, 693.133, /* Nose_Right_3 */
    535.28, 731.222, /* Nose_Right_Wing */
    514.973, 761.179, /* Nose_Right_Nostril_1 */
    501.567, 749.608, /* Nose_Right_Nostril_2 */
    488.16, 767.993, /* Nose_Right_Nostril_3 */
    432.411, 569.9, /* Nose_Left_Top */
    379.503, 724.559, /* Nose_Left_Wing */
    406.319, 688.777, /* Nose_Left_3 */
    423.001, 654.897, /* Nose_Left_2 */
    429.513, 614.416, /* Nose_Left_1 */
    458.846, 564.011, /* Nose_Nason_0 */
    459.208, 685.42, /* Nose_Tip_0 */
    398.062, 759.421, /* Nose_Left_Nostril_1 */
    413.058, 746.609, /* Nose_Left_Nostril_2 */
    428.053, 768.659, /* Nose_Left_Nostril_3 */
    456.921, 801.942, /* Nose_Subnasale_0 */
    348.16, 852.791, /* Lips_Upper_Left_Corner */
    388.868, 836.784, /* Lips_Upper_Outer_1 */
    428.59, 826.819, /* Lips_Upper_Outer_2 */
    546.116, 860.882, /* Lips_Upper_Right_Corner */
    514.036, 840.242, /* Lips_Upper_Outer_5 */
    477.717, 827.991, /* Lips_Upper_Outer_4 */
    451.802, 852.366, /* Lips_Upper_Inner_3 */
    453.389, 833.483, /* Lips_Upper_Outer_3 */
    382.89, 849.763, /* Lips_Upper_Inner_1 */
    416.062, 849.17, /* Lips_Upper_Inner_2 */
    513.528, 854.64, /* Lips_Upper_Inner_5 */
    482.565, 851.606, /* Lips_Upper_Inner_4 */
    549.428, 862.034, /* Lips_Lower_Right_Corner */
    405.162, 874.263, /* Lips_Lower_Outer_2 */
    449.571, 881.455, /* Lips_Lower_Outer_3 */
    488.151, 878.513, /* Lips_Lower_Outer_4 */
    518.126, 873.221, /* Lips_Lower_Outer_5 */
    414.122, 849.477, /* Lips_Lower_Inner_2 */
    485.715, 851.869, /* Lips_Lower_Inner_4 */
    516.507, 857.009, /* Lips_Lower_Inner_5 */
    377.873, 850.537, /* Lips_Lower_Inner_1 */
    344.946, 852.172, /* Lips_Lower_Left_Corner */
    451.407, 853.249, /* Lips_Lower_Inner_3 */
    370.817, 864.544, /* Lips_Lower_Outer_1 */
    242.29, 1023.15, /* Neck_Left_Out_2 */
    248.144, 936.263, /* Neck_Left_Top */
    246.622, 978.288, /* Neck_Left_Out_1 */
    239.478, 1068.01, /* Neck_Left_Bottom */
    661.639, 967.901, /* Neck_Right_Top */
    651.17, 1094.67, /* Neck_Right_Bottom */
    653.322, 1052.28, /* Neck_Right_Out_2 */
    657.303, 1010.08 /* Neck_Right_Out_1 */ };
  float marker_confidences[] = {
    1, /* Skull_Top_Right_2 */
    1, /* Skull_Top_Right_1 */
    1, /* Skull_Top_Left_End */
    1, /* Skull_Top_Right_End */
    1, /* Skull_Top_Left_2 */
    1, /* Skull_Top_Left_1 */
    1, /* Skull_Top_Vertex */
    1, /* Hairline_Right_End */
    1, /* Hairline_Right_3 */
    1, /* Hairline_Right_1 */
    1, /* Hairline_Trichion */
    1, /* Hairline_Left_1 */
    1, /* Hairline_Left_3 */
    1, /* Hairline_Left_End */
    1, /* Hairline_Left_2 */
    1, /* Hairline_Right_2 */
    1, /* Ear_Left_Vertex */
    1, /* Ear_Left_Obs_Vertex */
    1, /* Ear_Left_Obs */
    1, /* Ear_Left_Obi */
    1, /* Ear_Left_Lobe */
    1, /* Ear_Left_Out_1 */
    1, /* Ear_Left_Out_4 */
    1, /* Ear_Left_Out_2 */
    1, /* Ear_Left_Out_3 */
    1, /* Ear_Left_Out_5 */
    1, /* Ear_Right_Out_4 */
    1, /* Ear_Right_Lobe */
    1, /* Ear_Right_Obs_Vertex */
    1, /* Ear_Right_Vertex */
    1, /* Ear_Right_Out_2 */
    1, /* Ear_Right_Obi */
    1, /* Ear_Right_Obs */
    1, /* Ear_Right_Out_1 */
    1, /* Ear_Right_Out_3 */
    1, /* Ear_Right_Out_5 */
    1, /* Jaw_Gnathon */
    1, /* Jaw_Right_1 */
    1, /* Jaw_Left_1 */
    1, /* Jaw_Left_2 */
    1, /* Jaw_Left_3 */
    1, /* Jaw_Left_4 */
    1, /* Jaw_Right_2 */
    1, /* Jaw_Right_3 */
    1, /* Jaw_Right_4 */
    1, /* Jaw_Left_Obi */
    1, /* Jaw_Right_Obi */
    1, /* Jaw_Left_5 */
    1, /* Jaw_Right_5 */
    1, /* Eyebrow_Left_Upper_3 */
    1, /* Eyebrow_Left_Upper_2 */
    1, /* Eyebrow_Left_Upper_1 */
    1, /* Eyebrow_Left_Lower_1 */
    1, /* Eyebrow_Left_Lower_2 */
    1, /* Eyebrow_Left_Lower_3 */
    1, /* Eyebrow_Left_Outer_Corner */
    1, /* Eyebrow_Left_Inner_Corner */
    1, /* Eyebrow_Right_Upper_1 */
    1, /* Eyebrow_Right_Upper_2 */
    1, /* Eyebrow_Right_Upper_3 */
    1, /* Eyebrow_Right_Lower_1 */
    1, /* Eyebrow_Right_Lower_2 */
    1, /* Eyebrow_Right_Lower_3 */
    1, /* Eyebrow_Right_Inner_Corner */
    1, /* Eyebrow_Right_Outer_Corner */
    1, /* Eye_Left_Inner_Iris_0 */
    1, /* Eye_Left_Center_0 */
    1, /* Eye_Left_Exocanthion */
    1, /* Eye_Left_Endocanthion */
    1, /* Eye_Left_Top_Lid_1 */
    1, /* Eye_Left_Top_Lid_5 */
    1, /* Eye_Left_Bottom_Lid_5 */
    1, /* Eye_Left_Bottom_Lid_3 */
    1, /* Eye_Left_Bottom_Lid_1 */
    1, /* Eye_Left_Top_Lid_3 */
    1, /* Eye_Left_Outer_Iris_0 */
    1, /* Eye_Left_Pupil_Center_0 */
    1, /* Eye_Left_Top_Lid_4 */
    1, /* Eye_Left_Top_Lid_2 */
    1, /* Eye_Left_Bottom_Lid_2 */
    1, /* Eye_Left_Bottom_Lid_4 */
    1, /* Eye_Left_Top_Fold_2 */
    1, /* Eye_Left_Top_Fold_3 */
    1, /* Eye_Left_Top_Fold_4 */
    1, /* Eye_Left_Top_Fold_5 */
    1, /* Eye_Left_Top_Fold_6 */
    1, /* Eye_Left_Top_Fold_7 */
    1, /* Eye_Left_Top_Fold_Inner_Corner */
    1, /* Eye_Left_Top_Fold_1 */
    1, /* Eye_Left_Top_Fold_Outer_Corner */
    1, /* Eye_Left_Top_Lid_6 */
    1, /* Eye_Left_Top_Lid_7 */
    1, /* Eye_Left_Bottom_Lid_6 */
    1, /* Eye_Left_Bottom_Lid_7 */
    1, /* Eye_Right_Exocanthion */
    1, /* Eye_Right_Endocanthion */
    1, /* Eye_Right_Bottom_Lid_5 */
    1, /* Eye_Right_Bottom_Lid_3 */
    1, /* Eye_Right_Bottom_Lid_1 */
    1, /* Eye_Right_Top_Lid_1 */
    1, /* Eye_Right_Inner_Iris_0 */
    1, /* Eye_Right_Top_Lid_5 */
    1, /* Eye_Right_Top_Lid_3 */
    1, /* Eye_Right_Outer_Iris_0 */
    1, /* Eye_Right_Pupil_Center_0 */
    1, /* Eye_Right_Center_0 */
    1, /* Eye_Right_Top_Lid_2 */
    1, /* Eye_Right_Top_Lid_4 */
    1, /* Eye_Right_Bottom_Lid_2 */
    1, /* Eye_Right_Bottom_Lid_4 */
    1, /* Eye_Right_Top_Fold_6 */
    1, /* Eye_Right_Top_Fold_5 */
    1, /* Eye_Right_Top_Fold_4 */
    1, /* Eye_Right_Top_Fold_3 */
    1, /* Eye_Right_Top_Fold_2 */
    1, /* Eye_Right_Top_Fold_7 */
    1, /* Eye_Right_Top_Fold_Inner_Corner */
    1, /* Eye_Right_Top_Fold_1 */
    1, /* Eye_Right_Top_Fold_Outer_Corner */
    1, /* Eye_Right_Top_Lid_6 */
    1, /* Eye_Right_Top_Lid_7 */
    1, /* Eye_Right_Bottom_Lid_6 */
    1, /* Eye_Right_Bottom_Lid_7 */
    1, /* Nose_Midpoint */
    1, /* Nose_Right_Top */
    1, /* Nose_Right_2 */
    1, /* Nose_Right_1 */
    1, /* Nose_Right_3 */
    1, /* Nose_Right_Wing */
    1, /* Nose_Right_Nostril_1 */
    1, /* Nose_Right_Nostril_2 */
    1, /* Nose_Right_Nostril_3 */
    1, /* Nose_Left_Top */
    1, /* Nose_Left_Wing */
    1, /* Nose_Left_3 */
    1, /* Nose_Left_2 */
    1, /* Nose_Left_1 */
    1, /* Nose_Nason_0 */
    1, /* Nose_Tip_0 */
    1, /* Nose_Left_Nostril_1 */
    1, /* Nose_Left_Nostril_2 */
    1, /* Nose_Left_Nostril_3 */
    1, /* Nose_Subnasale_0 */
    1, /* Lips_Upper_Left_Corner */
    1, /* Lips_Upper_Outer_1 */
    1, /* Lips_Upper_Outer_2 */
    1, /* Lips_Upper_Right_Corner */
    1, /* Lips_Upper_Outer_5 */
    1, /* Lips_Upper_Outer_4 */
    1, /* Lips_Upper_Inner_3 */
    1, /* Lips_Upper_Outer_3 */
    1, /* Lips_Upper_Inner_1 */
    1, /* Lips_Upper_Inner_2 */
    1, /* Lips_Upper_Inner_5 */
    1, /* Lips_Upper_Inner_4 */
    1, /* Lips_Lower_Right_Corner */
    1, /* Lips_Lower_Outer_2 */
    1, /* Lips_Lower_Outer_3 */
    1, /* Lips_Lower_Outer_4 */
    1, /* Lips_Lower_Outer_5 */
    1, /* Lips_Lower_Inner_2 */
    1, /* Lips_Lower_Inner_4 */
    1, /* Lips_Lower_Inner_5 */
    1, /* Lips_Lower_Inner_1 */
    1, /* Lips_Lower_Left_Corner */
    1, /* Lips_Lower_Inner_3 */
    1, /* Lips_Lower_Outer_1 */
    1, /* Neck_Left_Out_2 */
    1, /* Neck_Left_Top */
    1, /* Neck_Left_Out_1 */
    1, /* Neck_Left_Bottom */
    1, /* Neck_Right_Top */
    1, /* Neck_Right_Bottom */
    1, /* Neck_Right_Out_2 */
    1 /* Neck_Right_Out_1 */ };
	char *error_message, *image_array, *image_ptr, *texture_array,
		*hair_texture_array, *distorted_background_array;
	char *eye_marker_names[] = {"eyeball_origin_left",
										 "eyeball_origin_right"};
	float marker_fitted_3d_positions[3 * 9],
		eyeball_fitted_3d_positions[3 * 2],
		error, eye_point[3], interest_point[3], up_vector[3], 
		*vertex_3d_locations, view_angle;
	int i, j, number_of_modes, number_of_vertices,
		pf_job_id;
	struct Obj hair_obj, obj;

	ALLOCATE(image_array, char, 3 * IMAGE_WIDTH * IMAGE_HEIGHT);
	ALLOCATE(texture_array, char, 3 * TEXTURE_WIDTH * TEXTURE_HEIGHT);
	ALLOCATE(hair_texture_array, char, 4 * TEXTURE_WIDTH * TEXTURE_HEIGHT);
	ALLOCATE(distorted_background_array, char, 3 * TEXTURE_WIDTH * TEXTURE_HEIGHT);

	if (PF_SUCCESS_RC == pf_specify_paths("/home/blackett/lifefx/photoface/", "/home/blackett/lifefx/photoface/"))
	{
		printf("Completed pf_specify_paths.\n");
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_specify_paths failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_setup("rachelv_r05m", "", &pf_job_id))
	{
		printf("Completed pf_setup.\n");
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_setup failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_specify_markers(pf_job_id, number_of_markers, marker_names,
			 marker_2d_positions, marker_confidences))
	{
		printf("Completed pf_specify_markers.\n");
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_specify_markers failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_view_align(pf_job_id, &error))
	{
		printf("Completed pf_view_align.\n");
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_view_align failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_get_view(pf_job_id, eye_point, interest_point, up_vector, &view_angle))
	{
		printf("Got view parameters.\n");
		printf("   Eye point %f %f %f.\n", eye_point[0], eye_point[1], eye_point[2]);
		printf("   Interest point %f %f %f.\n", interest_point[0], interest_point[1],
			interest_point[2]);
		printf("   Up vector %f %f %f.\n", up_vector[0], up_vector[1], up_vector[2]);
		printf("   View angle %f.\n", view_angle);
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_view failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_fit(pf_job_id, &error))
	{
		printf("Completed pf_fit.\n");
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_view failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_get_head_model(pf_job_id, &(obj.number_of_vertices), &(obj.vertex_3d_locations),
		&(obj.number_of_texture_vertices), &(obj.texture_vertex_3d_locations),
		&(obj.number_of_triangles), &(obj.triangle_vertices),
	  &(obj.triangle_texture_vertices)))
	{
		printf("Completed pf_get_head_model. vertices %d, %d, triangles %d\n",
			obj.number_of_vertices, obj.number_of_texture_vertices, obj.number_of_triangles);
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_head_model failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	/* Put something into the image array */
	image_ptr = image_array;
	for (i = 0 ; i < IMAGE_HEIGHT ; i++)
	{
		for (j = 0 ; j < IMAGE_WIDTH ; j++)
		{
			*image_ptr = 255.0 * (float)j / (float)IMAGE_WIDTH;
			image_ptr++;
			*image_ptr = 255.0 * (float)i /  (float)IMAGE_HEIGHT;
			image_ptr++;
			*image_ptr = 128.0;
			image_ptr++;
		}
	}
	
	if (PF_SUCCESS_RC == pf_get_marker_fitted_positions(pf_job_id, number_of_markers, 
		marker_names, marker_fitted_3d_positions))
	{
		printf("Completed pf_get_marker_fitted_positions.\n");
		printf("Fitted positions:\n");
		for (i = 0 ; i < number_of_markers ; i++)
		{
			printf("   %s:  %f %f %f\n", marker_names[i],
				marker_fitted_3d_positions[3 * i], marker_fitted_3d_positions[3 * i + 1], 
				marker_fitted_3d_positions[3 * i + 2]);
		}
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_marker_fitted_positions failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_get_marker_fitted_positions(pf_job_id, 2, 
		eye_marker_names, eyeball_fitted_3d_positions))
	{
		printf("Completed pf_get_marker_fitted_positions for eyeballs.\n");
		printf("Eyeball Fitted positions:\n");
		for (i = 0 ; i < 2 ; i++)
		{
			printf("   %s:  %f %f %f\n", eye_marker_names[i],
				eyeball_fitted_3d_positions[3 * i], eyeball_fitted_3d_positions[3 * i + 1], 
				eyeball_fitted_3d_positions[3 * i + 2]);
		}
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_marker_fitted_positions failed for eyeballs: %s\n",
			error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_get_basis(pf_job_id, &number_of_modes, &number_of_vertices,
		&vertex_3d_locations))
	{
		printf("Completed pf_get_basis.\n");
		printf("Basis:  Modes %d, vertices %d\n", number_of_modes,
			number_of_vertices);
		printf("Basis[10][10]:  %f\n", vertex_3d_locations[10 * 
			3 * number_of_vertices + 10]);
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_basis failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_specify_image(pf_job_id, IMAGE_WIDTH, IMAGE_HEIGHT, PF_RGB_IMAGE, image_array))
	{
		printf("Completed pf_specify_image.\n");
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_specify_image failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_get_texture(pf_job_id, TEXTURE_WIDTH, TEXTURE_HEIGHT, PF_RGB_IMAGE, 
			 texture_array))
	{
		printf("Completed pf_get_texture.\n");
		printf("Texture[30][30]: %d %d %d\n", texture_array[3 * 30 * TEXTURE_WIDTH + 3 * 30],
			texture_array[3 * 30 * TEXTURE_WIDTH + 3 * 30 + 1], texture_array[3 * 30 * TEXTURE_WIDTH + 3 * 30 + 2]);
		printf("Texture[70][70]: %d %d %d\n", texture_array[3 * 70 * TEXTURE_WIDTH + 3 * 70],
			texture_array[3 * 70 * TEXTURE_WIDTH + 3 * 70 + 1], texture_array[3 * 70 * TEXTURE_WIDTH + 3 * 70 + 2]);
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_texture failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	/* Put something into the image array */
	image_ptr = image_array;
	for (i = 0 ; i < IMAGE_HEIGHT ; i++)
	{
		for (j = 0 ; j < IMAGE_WIDTH ; j++)
		{
			*image_ptr = (i * 255 / IMAGE_HEIGHT + 128) % 256;
			image_ptr++;
			*image_ptr = *(image_ptr-1);
			image_ptr++;
			*image_ptr = *(image_ptr-1);
			image_ptr++;
		}
	}

	if (PF_SUCCESS_RC == pf_get_hair_model(pf_job_id, &(hair_obj.number_of_vertices), &(hair_obj.vertex_3d_locations),
		&(hair_obj.number_of_texture_vertices), &(hair_obj.texture_vertex_3d_locations),
		&(hair_obj.number_of_triangles), &(hair_obj.triangle_vertices),
	  &(hair_obj.triangle_texture_vertices)))
	{
		printf("Completed pf_get_hair_model. vertices %d, %d, triangles %d\n",
			hair_obj.number_of_vertices, hair_obj.number_of_texture_vertices, hair_obj.number_of_triangles);
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_hair_model failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_specify_hair_mask(pf_job_id, IMAGE_WIDTH, IMAGE_HEIGHT, PF_RGB_IMAGE, image_array))
	{
		printf("Completed pf_specify_hair_mask.\n");
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_specify_hair_mask failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}		
	
	if (PF_SUCCESS_RC == pf_get_hair_texture(pf_job_id, TEXTURE_WIDTH, TEXTURE_HEIGHT, 
			 PF_RGBA_IMAGE, hair_texture_array))
	{
		printf("Completed pf_get_hair_texture.\n");
		printf("Hair Texture[30][30]: %d %d %d\n", hair_texture_array[4 * 30 * TEXTURE_WIDTH + 4 * 30],
			hair_texture_array[4 * 30 * TEXTURE_WIDTH + 4 * 30 + 1], hair_texture_array[4 * 30 * TEXTURE_WIDTH + 4 * 30 + 2]);
		printf("Hair Texture[70][70]: %d %d %d\n", hair_texture_array[4 * 70 * TEXTURE_WIDTH + 4 * 70],
			hair_texture_array[4 * 70 * TEXTURE_WIDTH + 4 * 70 + 1], hair_texture_array[4 * 70 * TEXTURE_WIDTH + 4 * 70 + 2]);
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_hair_texture failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	if (PF_SUCCESS_RC == pf_get_distorted_background(pf_job_id, TEXTURE_WIDTH, TEXTURE_HEIGHT, 
			 PF_RGB_IMAGE, distorted_background_array))
	{
		printf("Completed pf_get_distorted_background.\n");
		printf("Distorted Background[30][30]: %d %d %d\n", distorted_background_array[3 * 30 * TEXTURE_WIDTH + 3 * 30],
			distorted_background_array[3 * 30 * TEXTURE_WIDTH + 3 * 30 + 1], distorted_background_array[3 * 30 * TEXTURE_WIDTH + 3 * 30 + 2]);
		printf("Distorted Background[70][70]: %d %d %d\n", distorted_background_array[3 * 70 * TEXTURE_WIDTH + 3 * 70],
			distorted_background_array[3 * 70 * TEXTURE_WIDTH + 3 * 70 + 1], distorted_background_array[3 * 70 * TEXTURE_WIDTH + 3 * 70 + 2]);
	}
	else
	{
		pf_get_error_message(&error_message);
		fprintf(stderr, "ERROR: pf_get_distorted_background failed: %s\n", error_message);
		DEALLOCATE(error_message);
	}

	/* Write out the files */
	/* Most of the model comes from pf_get_head_model but the number of dynamic vertices
		comes from pf_get_basis */
	pf_write_head_model("test_photoface.obj", obj.number_of_vertices, number_of_vertices, 
		obj.vertex_3d_locations, obj.number_of_texture_vertices, obj.texture_vertex_3d_locations,
		obj.number_of_triangles, obj.triangle_vertices,
		obj.triangle_texture_vertices);
	
	pf_write_basis("test_photoface.basis" ,number_of_modes,
		number_of_vertices, vertex_3d_locations);

	pf_write_texture("test_photoface.jpg", TEXTURE_WIDTH, TEXTURE_HEIGHT, texture_array);

	pf_write_scene_graph("test_photoface_scenegraph.txt",
		eye_point, interest_point, up_vector, view_angle,
		eyeball_fitted_3d_positions, eyeball_fitted_3d_positions + 3);
	/* Now we could use these files to generate a head */

	/* Free up stuff we no longer need */
	DEALLOCATE(obj.vertex_3d_locations);
	DEALLOCATE(obj.texture_vertex_3d_locations);
	DEALLOCATE(obj.triangle_vertices);
	DEALLOCATE(obj.triangle_texture_vertices);
	DEALLOCATE(hair_obj.vertex_3d_locations);
	DEALLOCATE(hair_obj.texture_vertex_3d_locations);
	DEALLOCATE(hair_obj.triangle_vertices);
	DEALLOCATE(hair_obj.triangle_texture_vertices);
	DEALLOCATE(vertex_3d_locations);

	/* End this job */
	pf_close(pf_job_id);

	/* Free up the path memory */
	pf_specify_paths(NULL, NULL);

	DEALLOCATE(image_array);
	DEALLOCATE(texture_array);
	DEALLOCATE(hair_texture_array);
	DEALLOCATE(distorted_background_array);

#if defined (MEMORY_CHECKING)
	list_memory(/*count*/0, /*show_pointers*/1, /*increment_counter*/1, 
	  /*show_structures*/1);
#endif /* defined (MEMORY_CHECKING) */
}
