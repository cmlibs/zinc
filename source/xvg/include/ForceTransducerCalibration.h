/***********************************************************************
*
*  Name:          ForceTransducerCalibration.h
*
*  Author:        Paul Charette & Henri Brouwers
*
*  Last Modified:
*                 - Paul Charette, 7 April 1997
*                   Changed for Henri's updated coefficients.
*
*  Purpose:       Define model coefficients for conversion of Hall-effect
*                 voltage outputs to force. The order of the parameters
*                 in the 4 arrays MUST be the same as the numbering of the
*                 actuators on the biaxial rig.
*
***********************************************************************/

/* model : mN = a0 + a1*mV + a2*mV*mV */
static double mV_to_mN_a2[16] = {
  0.0004495, 0.0002664, 0.0007028, 0.0008666,
  0.0006156, 0.0004092, 0.0005398, 0.0008499,
  0.0005000, 0.0005813,	0.0003637, 0.0004210,
  0.0002235, 0.0004043, 0.0004406, 0.0003599};
static double mV_to_mN_a1[16] = {
  -3.4909, -3.7730, -4.1810, -4.8649,
  -3.7210, -3.3961, -3.9540, -4.2606,
  -4.0210, -4.4730, -4.2990, -4.1067,
  -3.9095, -4.2925, -3.9718, -3.6641};
static double mV_to_mN_a0[16] = {
  985.3, 903.4,  681.9,  698.3,
  727.0, 810.5,  813.0,  726.5,
  412.5, 390.0, 1075.8, 1064.4,
  918.1, 785.7,  616.4,  945.8};

/* output of the force transducers (in mV) with no force applied */
/* and no interaction from neighboring Hall-effect devices       */
static double NoForceOffsets_mV[16] = {
  293.2602, 243.6061, 167.7861, 147.2895,
  202.1431, 245.9320, 211.7238, 176.7366,
  103.9165,  88.1261, 255.7299, 266.3958,
  238.0587, 186.2755, 157.8593, 264.9422};


