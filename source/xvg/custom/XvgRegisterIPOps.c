/***********************************************************************
*
*  Name:          XvgregisterIPOps.c
*
*  Author:        Paul Charette
*
*  Last Modified: 26 of Nov 1994
*
*  Purpose:       Register IP operations into internal data structure
*                 so that they are made available to the editing window
*                 popup menu and the command line parser.
*
*       Example:  RegisterIPOp(name, fname,
*                              MenuID, RepsAllowedFlag, EdgeFixFlag,
*                              parm1name, parm1type,
*                              parm2name, parm2type, 
*                              ...        ...
*                              NULL);
*
*                 where : char *name      : string that appears in the menu
*                                           and tag for the command line
*                                           parser
*                         void (*fname)() : function call name
*                         int MenuID      : Submenu category ID
*                                           choices are:
*                                               BINARYOPS
*                                               DYNRANGEOPS
*                                               FFTOPS
*                                               FILEIOOPS
*                                               FILTEROPS
*                                               FITOPS
*                                               STEREOOPS
*                                               GRABBINGOPS
*                                               MISCOPS
*                                               MORPHOLOGICALOPS
*                                               RPNOPS
*                                               STACKOPS
*                                               STRAINOPS
*                                               VIDEOOPS
*                                               WRAPOPS
*                         boolean_t RepsAllowedFlag:
*                                           B_TRUE = allow repetitons of IP Op,
*                                           else set to B_FALSE
*                         boolean_t EdgeFixFlag:
*                                           B_TRUE = for neighborhood type IPOps,
*                                           else B_FALSE
*                 NULL terminate list of parameter description pairs:
*                         char *parm?name : parameter name that appear in the
*                                           editing window
*                         int parm?type   : parameter data type
*                                           (INTEGER_GT, DOUBLE_DT, STRING_DT)
*
*
*       The function calling sequence must be as follows:
*
*                 fname(double *inpixels, double *outpixels, int height,
*                       int width, void *ParmPtrs[])
*
*                 (look at the file imgproc.c)
*
*       The code for the new operation can be included in the file
*       "imgproc.c" or in a separate source file which must then be included
*       in the makefile "xvg.mk".
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

void RegisterIPOps(void)
{
  /* Image saving/loading IO Ops */
  RegisterIPOp("LoadImageFile", LoadImageFile, FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
#ifndef CMGUI
  RegisterIPOp("LoadMaskFile", LoadMaskFile, FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
  RegisterIPOp("SaveBSpline", SaveBSpline, FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
#endif
  RegisterIPOp("SaveDataSet", SaveDataSet, FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
#ifndef CMGUI
  RegisterIPOp("SaveDataSetByte", SaveDataSetByte, FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
  RegisterIPOp("SaveDataSetShort", SaveDataSetShort,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
  RegisterIPOp("SaveDataSetReal", SaveDataSetReal, FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
#endif
  RegisterIPOp("SaveTIFFColorFile", SaveTIFFColorFile,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
  RegisterIPOp("SaveTIFFGreyLevelFile", SaveTIFFGreyLevelFile,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
  RegisterIPOp("SaveTIFFGreyLevelFixedByteDRFile",
	       SaveTIFFGreyLevelFixedByteDRFile,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
#ifndef CMGUI
  RegisterIPOp("LoadArctanError", LoadArctanErrorImg,
	       FILEIOOPS, B_FALSE, B_FALSE, 
               NULL);
  RegisterIPOp("LoadArctanRadius", LoadArctanRadiusImg, FILEIOOPS,
	       B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Load2ICosImg", LoadCosImg, FILEIOOPS, B_FALSE, B_FALSE, 
               NULL);
#endif
  RegisterIPOp("LoadMask", LoadMask, FILEIOOPS,
	       B_FALSE, B_FALSE, NULL);
#ifndef CMGUI
  RegisterIPOp("Load2ISinImg", LoadSinImg, FILEIOOPS, B_FALSE, B_FALSE, 
               NULL);
#endif
  RegisterIPOp("ReadCMISSFieldSamplesFile", ReadCMISSFieldSamplesFile,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Input filename", STRING_DT,
	       "Field (X:0, Y:1)", INTEGER_DT,
	       "Interpolate (0/1)", INTEGER_DT,
	       NULL);
  RegisterIPOp("WriteCMISSFieldSamplesFile", WriteCMISSFieldSamplesFile,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Output fname matissa", STRING_DT,
	       "Decimation (none: 1)", INTEGER_DT,
	       "MM / pixel", DOUBLE_DT,
	       "pcx pcy", DOUBLE_ARRAY_DT,
	       NULL);
  RegisterIPOp("WriteCMISSNodesFile", WriteCMISSNodesFile,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Output fname matissa", STRING_DT,
	       NULL);
  RegisterIPOp("WriteCMISSPointsFile", WriteCMISSPointsFile,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Output fname matissa", STRING_DT,
	       "Data type (0:WXYZ, 1:Z only)", INTEGER_DT,
	       "Decimation (None:1)", INTEGER_DT,
	       "WThreshold MM/pix pcx pcy", DOUBLE_ARRAY_DT,
	       NULL);
  RegisterIPOp("WriteMatlabUShortField",
	       WriteMatlabUShortField,
	       FILEIOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT,
	       "Field (0/1)", INTEGER_DT,
	       "X Decimation (None = 1)", INTEGER_DT,
               NULL);

  /* stack manipulation Ops */
  RegisterIPOp("PopImage", PopImage, STACKOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("PushImage", PushImage, STACKOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("StackClear", StackClear, STACKOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("StackExchangePos", StackExchangePos, STACKOPS,
	       B_FALSE, B_FALSE, "pos 1", INTEGER_DT, "pos 2", INTEGER_DT,
	       NULL);
  RegisterIPOp("SwapXY", SwapXY, STACKOPS, B_FALSE, B_FALSE, NULL);

  /* grabbing ops */
#ifdef VCAA_CODE
/*
  RegisterIPOp("GrabStep", GrabStep, GRABBINGOPS, B_FALSE, B_FALSE,
	       "nsteps", INTEGER_DT, "delay (s)", INTEGER_DT, NULL);
*/
  RegisterIPOp("GrabFrame", GrabFrame, GRABBINGOPS, B_FALSE, B_FALSE,NULL);
/*
  RegisterIPOp("DoubleGrabSubtract", DoubleGrabSubtract, GRABBINGOPS,
	       B_FALSE, B_FALSE, "delay (s)", INTEGER_DT, NULL);
  RegisterIPOp("StepPlus", StepPlus, GRABBINGOPS, B_FALSE, B_FALSE,NULL);
  RegisterIPOp("StepMinus", StepMinus, GRABBINGOPS, B_FALSE, B_FALSE,NULL);
*/
#endif
#ifdef SGI
  RegisterIPOp("FrameGrab", FrameGrab, GRABBINGOPS, B_FALSE, B_FALSE,NULL);
  RegisterIPOp("NetworkGrab", NetworkGrab, GRABBINGOPS, B_FALSE, B_FALSE,
	       "Output directory (local dir ex: /usr/tmp/biaxial)", STRING_DT,
	       NULL);
#endif
  

  /* dynamic range ops */
#ifndef CMGUI
  RegisterIPOp("FreeDynamicRange", FreeDynamicRange, DYNRANGEOPS, B_FALSE, B_FALSE,
	       NULL);
  RegisterIPOp("FreezeDynamicRange", FreezeDynamicRange, DYNRANGEOPS, B_FALSE,
	       B_FALSE, NULL);
  RegisterIPOp("SetDynamicRange", SetDynamicRange, DYNRANGEOPS, B_FALSE, B_FALSE, 
	       "Minimum value", DOUBLE_DT, "Maximum value", DOUBLE_DT,
	       NULL);
#endif

  /* RPN operations */
  RegisterIPOp("+", RPNAdd, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("+C", RPNAddConstant, RPNOPS, B_FALSE, B_FALSE, "C", DOUBLE_DT,
	       NULL);
  RegisterIPOp("Subtract", RPNSubtract, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Round to integer", RPNRint, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("x", RPNMultiply, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("xC", RPNMultiplyConstant, RPNOPS, B_FALSE, B_FALSE, "C", DOUBLE_DT,
	       NULL);
  RegisterIPOp("1/x", RPNXInverse, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Divide", Divide, RPNOPS, B_FALSE, B_FALSE, "C", DOUBLE_DT,
	       NULL);
  RegisterIPOp("ModC", RPNModConstant, RPNOPS, B_FALSE, B_FALSE, "C", DOUBLE_DT,
	       NULL);
  RegisterIPOp("Sqr", Sqr, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Sqrt", Sqrt, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Sin", Sin, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Cos", Cos, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Asin", Asin, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Acos", Acos, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Atan", Atan, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Atan2", Atan2, RPNOPS, B_FALSE, B_FALSE, NULL);
#ifndef CMGUI
  RegisterIPOp("SpeckleAtan", SpeckleAtan, RPNOPS, B_FALSE, B_FALSE,
	       "atan2/atan (0/1)", INTEGER_DT, NULL);
#endif
  RegisterIPOp("Exp", Exp, RPNOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Log", Log, RPNOPS, B_FALSE, B_FALSE, NULL);

  /* morphological ops */
#if defined NAG
  RegisterIPOp("Contour", Contour, MORPHOLOGICALOPS, B_FALSE, B_FALSE,
	       "Close/Dilation Radius", INTEGER_DT,
	       "Close/Erosion Radius", INTEGER_DT,
	       "Open/Erosion Radius", INTEGER_DT,
	       "Open/Dilation Radius", INTEGER_DT,
	       NULL);
#endif
  RegisterIPOp("Dilation", Dilation, MORPHOLOGICALOPS, B_FALSE, B_FALSE,
	       "Disk Radius", INTEGER_DT, NULL);
#ifdef NAG
  RegisterIPOp("DilationFFT", DilationFFT, MORPHOLOGICALOPS, B_FALSE, B_FALSE,
	       "Disk Radius", INTEGER_DT, NULL);
  RegisterIPOp("Disk", Disk, MORPHOLOGICALOPS, B_FALSE, B_FALSE,
	       "x", INTEGER_DT, "y", INTEGER_DT, "Radius", INTEGER_DT,
	       NULL);
#endif
  RegisterIPOp("Erosion", Erosion, MORPHOLOGICALOPS, B_FALSE, B_FALSE,
	       "Disk Radius", INTEGER_DT, NULL);
#ifdef NAG
  RegisterIPOp("ErosionFFT", ErosionFFT, MORPHOLOGICALOPS,
	       B_FALSE, B_FALSE, "Radius", INTEGER_DT, NULL);
#endif

#if defined (NAG)
  /* frequency domain ops */
  RegisterIPOp("AutoCorrelation", AutoCorrelation, FFTOPS, B_FALSE, B_FALSE,
	       "Padding", INTEGER_DT, NULL);
  RegisterIPOp("CrossCorrelation", CrossCorrelation, FFTOPS, B_FALSE, B_FALSE,
	       "Padding", INTEGER_DT, NULL);
  RegisterIPOp("DiskCorrelation", DiskCorrelation, FFTOPS, B_FALSE, B_FALSE,
	       "Radius", INTEGER_DT, "Scale [0..1]", DOUBLE_DT,
	       NULL);
  RegisterIPOp("FFT", FFT, FFTOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("FFTInverse", FFTInverse, FFTOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("HomomorphicFilter", HighpassFilter, FFTOPS, B_FALSE, B_FALSE,
	       "Lower asymptote", DOUBLE_DT,
	       "Upper asymptote", DOUBLE_DT,
	       "Cutoff Radius", DOUBLE_DT, NULL);
  RegisterIPOp("InterpolateFFT", InterpolateFFT, FFTOPS, B_FALSE, B_FALSE,
	       "OutFname", STRING_DT,
	       "Height", INTEGER_DT,
	       "Width", INTEGER_DT, NULL);
  RegisterIPOp("LowpassFilter", LowpassFilter, FFTOPS, B_FALSE, B_FALSE,
	       "Cutoff Radius", DOUBLE_DT, NULL);
  RegisterIPOp("NotchFilter", NotchFilter, FFTOPS, B_FALSE, B_FALSE,
	       "dx", INTEGER_DT, "dy", INTEGER_DT, NULL);
/*
  RegisterIPOp("CustomFilter", CustomFilter, FFTOPS, B_FALSE, B_FALSE,
	       NULL);
*/
  RegisterIPOp("ScaleImageFFT", ScaleImageFFT, FFTOPS, B_FALSE, B_FALSE,
	       "Scale factor", DOUBLE_DT, NULL);
  RegisterIPOp("ShiftImageFFT", ShiftImageFFT, FFTOPS, B_FALSE, B_FALSE,
	       "dx", DOUBLE_DT, "dy", DOUBLE_DT, NULL);
  RegisterIPOp("WaveletTransform", WaveletTransform, FFTOPS, B_FALSE, B_FALSE,
	       "0:WT, 1:IWT", INTEGER_DT, NULL);
  RegisterIPOp("ApplyWindow", ApplyWindow, FFTOPS, B_FALSE, B_FALSE, NULL);
#endif

  /* space domain filters */
  RegisterIPOp("AveragingFilter", AveragingFilter, FILTEROPS, B_TRUE, B_FALSE,
	       "Kernel Size", INTEGER_DT, NULL);
  RegisterIPOp("Detrend", Detrend, FILTEROPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("DetrendXYZ", DetrendXYZ, FILTEROPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Laplacian", Laplacian, FILTEROPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("MedianFilter", MedianFilter, FILTEROPS, B_TRUE, B_FALSE,
	       "Kernel Size", INTEGER_DT, NULL);
  RegisterIPOp("MedianFilterByte", MedianFilterByte, FILTEROPS, B_TRUE, B_FALSE,
	       "Kernel Size", INTEGER_DT, NULL);
  RegisterIPOp("RegionAveragingFilter", RegionAveragingFilter,
	       FILTEROPS, B_TRUE, B_FALSE, "Kernel Size", INTEGER_DT,
	       NULL);
  RegisterIPOp("Sharpen", Sharpen,
	       FILTEROPS, B_TRUE, B_FALSE, "Amplification", DOUBLE_DT,
	       NULL);
  RegisterIPOp("WeightedAveragingFilter", WeightedAveragingFilter,
	       FILTEROPS, B_FALSE, B_FALSE,
	       "Kernel Size", INTEGER_DT, NULL);

#ifndef CMGUI
  /* fitting */
#ifdef NAG
  RegisterIPOp("BSplines", BSplines, FITOPS, B_FALSE, B_FALSE,
	       "N interior knots", INTEGER_DT, NULL);
  RegisterIPOp("BSplinesAuto", BSplinesAuto, FITOPS, B_FALSE, B_FALSE,
	       "Smoothness factor", DOUBLE_DT,
	       "Type(0:scat/1:grd)", INTEGER_DT, NULL);
  RegisterIPOp("ChebyshevSurfaceFit", ChebyshevSurfaceFit,FITOPS, B_FALSE,
	       B_FALSE, "Order", INTEGER_DT, NULL);
#endif /* NAG */
  
  RegisterIPOp("BuildInterpolatedImage", BuildInterpolatedImage,
	       FITOPS, B_FALSE, B_FALSE,
	       "InFilename", STRING_DT, "E Coords (0/1)", INTEGER_DT, NULL);
  RegisterIPOp("CompDispField", CompDispField, FITOPS, B_FALSE, B_FALSE,
	       "Spacing", INTEGER_DT, "Scale", DOUBLE_DT,
	       "Fixed pt X", INTEGER_DT, "Fixed pt Y", INTEGER_DT,
	       NULL);
  RegisterIPOp("ComputeMarkerPath", ComputeMarkerPath, FITOPS, B_FALSE, B_FALSE,
	       "x0", INTEGER_DT, "y0", INTEGER_DT,
	       "FE filename", STRING_DT, "OutFilename", STRING_DT,
	       NULL);
  RegisterIPOp("CompStrainField", CompStrainField, FITOPS, B_FALSE, B_FALSE,
	       "Spacing", INTEGER_DT, "Scale", DOUBLE_DT,
	       "Strain step", INTEGER_DT, "PStrain fname", STRING_DT, NULL);
  RegisterIPOp("FECompute", FECompute, FITOPS, B_FALSE, B_FALSE,
	       "Fit LN/LN-NL (0/1)", INTEGER_DT,
	       "FO Threshold [0..1]", DOUBLE_DT,
	       "Outfiles prefix", STRING_DT,
	       "Field modl LN/CU (0/1)", INTEGER_DT,
	       NULL);
#ifdef FE_COMPUTE_DISPLACED_NODES_SPECKLE
  RegisterIPOp("FEComputeDisplacedNodes", FEComputeDisplacedNodes,
	       FITOPS, B_FALSE, B_FALSE,
	       "InFilename", STRING_DT, "OutFilename", STRING_DT, 
	       "Field modl LN/CU (0/1)", INTEGER_DT,
	       "Rotation angle (degs)", DOUBLE_DT,
	       NULL);
#endif
  RegisterIPOp("FEComputeDispNodesXCorr", FEComputeDispNodesXCorr,
	       FITOPS, B_FALSE, B_FALSE,
	       "Node filename", STRING_DT,
	       "InFilename", STRING_DT,
	       "OutFilename", STRING_DT, 
	       "MM/pix lags kernw pcx pcy npx npy ff", DOUBLE_ARRAY_DT,
	       NULL);

  RegisterIPOp("FEStrain", FEStrain, FITOPS, B_FALSE, B_FALSE,
	       "InFilename", STRING_DT, "Index", INTEGER_DT,
	       "Scale", DOUBLE_DT, "GPt Only (0:1)", INTEGER_DT,
	       NULL);
  RegisterIPOp("GenerateSampleFile", GenerateSampleFile, FITOPS, B_FALSE, B_FALSE,
	       "OutFilename", STRING_DT, "Spacing", INTEGER_DT,  NULL);
  RegisterIPOp("LoadMesh", LoadMesh, FITOPS, B_FALSE, B_FALSE,
	       "InFilename", STRING_DT, "Index", INTEGER_DT,
	       "Load PixelSpacing (0/1)", INTEGER_DT,
	       "Load MeshTemplate (0/1)", INTEGER_DT,
	       NULL);
  RegisterIPOp("PlaneFit", PlaneFit, FITOPS, B_FALSE, B_FALSE,
	       "Kernel Size", INTEGER_DT, "Gradients (0/1)", INTEGER_DT,
	       NULL);
  RegisterIPOp("RegionMaskCreation", RegionMaskCreation,
	       FITOPS, B_FALSE, B_FALSE, "Region size threshold", INTEGER_DT,
	       NULL);
#ifdef NAG
  RegisterIPOp("PolynomialFit", PolynomialFit, FITOPS, B_FALSE, B_FALSE, 
	       "Order", INTEGER_DT, NULL);
  RegisterIPOp("PowerSeriesModelEval", PowerSeriesModelEval,
	       FITOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
  RegisterIPOp("SpeckleAutoCorrelationFit", SpeckleAutoCorrelationFit, FITOPS,
	       B_FALSE, B_FALSE,
	       "kernel width", INTEGER_DT,
	       "soln upper bound", DOUBLE_DT,
	       "soln tolerance", DOUBLE_DT,
	       "model fit? (0/1)", INTEGER_DT,
	       NULL);
  RegisterIPOp("SpeckleAutoCorrelationTest", SpeckleAutoCorrelationTest,
	       FITOPS, B_FALSE, B_FALSE,
	       "tolerance", DOUBLE_DT,
	       "speckle size", DOUBLE_DT,
	       "lower bound", DOUBLE_DT,
	       "upper bound", DOUBLE_DT,
	       NULL);
#endif /* NAG */
  RegisterIPOp("VAF", VAFCompute, FITOPS, B_FALSE, B_FALSE, NULL);

  /* wrapping ops */
  RegisterIPOp("FixHook", FixHook, WRAPOPS, B_FALSE, B_FALSE,
	       "x", INTEGER_DT, "y", INTEGER_DT, "Radius", INTEGER_DT,
	       NULL);
#ifdef NAG
  RegisterIPOp("NonLinearLSPhaseFit", NonLinearLSPhaseFit, WRAPOPS,
	       B_FALSE, B_FALSE,
	       "SamplingPeriod", INTEGER_DT, "Order", INTEGER_DT,
	       "ModVal (0:No mod)", DOUBLE_DT,
	       NULL);
#endif /* NAG */
  RegisterIPOp("RegionBoundaryNormals", RegionBoundaryNormals,
	       WRAPOPS, B_FALSE, B_FALSE,
	       "Sampling period", INTEGER_DT,
	       "Window size", INTEGER_DT, NULL);
  RegisterIPOp("RegionExpansion", RegionExpansion, WRAPOPS,
	       B_FALSE, B_FALSE, NULL);
  RegisterIPOp("RegionFillSegmentation", RegionFillSegmentation,
	       WRAPOPS, B_FALSE, B_FALSE, "Region size threshold", INTEGER_DT,
	       "Expand (0/1)", INTEGER_DT,
	       NULL);
  RegisterIPOp("Residues", Residues, WRAPOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Unwrap", Unwrap, WRAPOPS, B_FALSE, B_FALSE,
	       "Threshold (0..1)", DOUBLE_DT, NULL);
  RegisterIPOp("Wrap", Wrap, WRAPOPS, B_FALSE, B_FALSE, NULL);
#endif /* CMGUI */

  /* stereo operations */
  RegisterIPOp("Build3DConeObject", Build3DConeObject,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Output filename prefix", STRING_DT,
	       "Base radius (mm)", DOUBLE_DT,
               "Elevation (degree)", DOUBLE_DT, 
	       "Camera sep (mm)", DOUBLE_DT,
	       NULL);
  RegisterIPOp("Build3DHemisphere", Build3DHemisphere,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Output filename prefix", STRING_DT,
	       "radius (mm)", DOUBLE_DT,
	       "Camera sep (mm)", DOUBLE_DT,
	       NULL);
  RegisterIPOp("Build3DPlaneObject", Build3DPlaneObject,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Output filename prefix", STRING_DT,
	       "XRot (degrees)", DOUBLE_DT,
               "YRot (degrees)", DOUBLE_DT, 
	       "Camera separation (mm)", DOUBLE_DT,
	       NULL);
  RegisterIPOp("GlobalPlaneFit", GlobalPlaneFit, STEREOOPS, B_FALSE, B_FALSE,
	       "DX (0:1)", DOUBLE_DT, "Fit type (0,1,2)", INTEGER_DT,
	       NULL);

  RegisterIPOp("MeanUVW", MeanUVW, STEREOOPS, B_FALSE, B_FALSE,
	       "Threshold", DOUBLE_DT, NULL);
  RegisterIPOp("OpticalFlow", OpticalFlow,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Image listing filename", STRING_DT,
	       "Node parameter file", STRING_DT,
	       "Xcorr parameter filename", STRING_DT,
	       "nx ny StartI StopI FitR MaskR O XcorrEn", DOUBLE_ARRAY_DT,
	       NULL);
  RegisterIPOp("OpticalFlowFieldFilter", OpticalFlowFieldFilter,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Input filename", STRING_DT,
	       "Unused", STRING_DT,
	       "Write intermediate files", INTEGER_DT,
	       "GenMsk LplT DNT FtpRad", DOUBLE_ARRAY_DT,
	       NULL);
  RegisterIPOp("OpticalFlowNodeCMFilter", OpticalFlowNodeCMFilter,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Input filename", STRING_DT,
	       "Output filename", STRING_DT,
	       "Min pixel Only (0/1)", INTEGER_DT,
	       NULL);
  RegisterIPOp("OpticalFlowLoadNodes", OpticalFlowLoadNodes,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT,
	       "Display Indicies (0:No)", INTEGER_DT,
	       NULL);
  RegisterIPOp("OpticalFlowMakeMovie", OpticalFlowMakeMovie,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Filename list file", STRING_DT,
	       "Output directory", STRING_DT,
	       "Flip vertically (0/1)", INTEGER_DT,
	       NULL);
  RegisterIPOp("PatchXCorr", PatchXCorr,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Output filename prefix", STRING_DT,
	       "Current img X scan start", INTEGER_DT,
	       "Current img Y scan start", INTEGER_DT,
	       "Xo Yo Xx Yx lg d kw R r st rt", DOUBLE_ARRAY_DT,
	       NULL);
  RegisterIPOp("PatchXCorrByAreaParallel", PatchXCorrByAreaParallel,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Output filename prefix", STRING_DT,
	       "Child process in X", INTEGER_DT,
	       "Child process in Y", INTEGER_DT,
	       "Lags FFTKernel RMax Dec SE_Flag", DOUBLE_ARRAY_DT,
	       NULL);
  RegisterIPOp("PatchXCorrByAreaHierarchical", PatchXCorrByAreaHierarchical,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Output filename prefix", STRING_DT,
	       "Child process in X", INTEGER_DT,
	       "Child process in Y", INTEGER_DT,
	       "Parameter filename", STRING_DT,
	       NULL);
  RegisterIPOp("PatchXCorrByLine", PatchXCorrByLine,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "Output filename prefix", STRING_DT,
	       "Lags", INTEGER_DT,
	       "YStart1 (-1 = auto-calc)", INTEGER_DT,
	       "YExtent (-1 = auto-calc)", INTEGER_DT,
	       NULL);
  RegisterIPOp("SequenceTrackPoints", SequenceTrackPoints,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "List of RGB files filename", STRING_DT,
	       "Parameter filename", STRING_DT,
	       "Nodes to track fname (or ALL)", STRING_DT,
	       "I0 I1 XRMAX BUGT OBT ST CRT", DOUBLE_ARRAY_DT,
	       NULL);
  RegisterIPOp("InterpolateSequence", InterpolateSequence,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "List of 2d files filename", STRING_DT,
	       "Outfilename mantissa", STRING_DT,
	       "UNUSED", STRING_DT,
	       "IMG0 IMG1", DOUBLE_ARRAY_DT,
	       NULL);
  RegisterIPOp("ConvertExNodeFileToMatlab", ConvertExNodeFileToMatlab,
	       STEREOOPS, B_FALSE, B_FALSE,
	       "List of exnode files filename", STRING_DT,
	       "Single sided ? (No:0)", INTEGER_DT,
	       NULL);
  
  /* binary image ops */
  RegisterIPOp("OR", OR, BINARYOPS, B_FALSE, B_FALSE, NULL);
#ifndef CMGUI
  RegisterIPOp("Skeleton", Skeleton, BINARYOPS, B_FALSE, B_FALSE,
	       "Minumum line length", INTEGER_DT,
	       "Maximum gap length", INTEGER_DT, NULL);
  /* misc image processing Ops */
  RegisterIPOp("AutoStereogram", RandomDotStereogram, MISCOPS,
	       B_FALSE, B_FALSE,"Mask (nbits)", INTEGER_DT,
	       "Mu", DOUBLE_DT, "DPI", DOUBLE_DT, NULL);
#endif /* CMGUI */
  RegisterIPOp("Binarize", Binarize, MISCOPS, B_FALSE, B_FALSE,
	       "Threshold", DOUBLE_DT, NULL);
  RegisterIPOp("BracketSD", BracketSD, MISCOPS, B_FALSE, B_FALSE,
	       "Scale", DOUBLE_DT, NULL);
#ifndef CMGUI
  RegisterIPOp("BandZero", BandZero, MISCOPS, B_FALSE, B_FALSE,
	       "Fraction [0..1]", DOUBLE_DT, NULL);
  RegisterIPOp("Center", Center, MISCOPS, B_FALSE, B_FALSE, NULL);
#endif /* CMGUI */
  RegisterIPOp("CaptureOverlay", CaptureOverlay, MISCOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("CreateNoiseImage", CreateNoiseImage, MISCOPS, B_FALSE, B_FALSE,
	       NULL);
  RegisterIPOp("CreateSpeckleImage", CreateSpeckleImage, MISCOPS, B_FALSE, B_FALSE,
	       "X period", INTEGER_DT, "Y period", INTEGER_DT,
	       "Noise Amplitude", DOUBLE_DT, NULL);
  RegisterIPOp("Crop", Crop, MISCOPS, B_FALSE, B_FALSE,
	       "X min", INTEGER_DT, "X max", INTEGER_DT,
	       "Y min", INTEGER_DT, "Y max", INTEGER_DT, NULL);
  RegisterIPOp("MaskOut", MaskOut, MISCOPS, B_FALSE, B_FALSE,
	       NULL);
  RegisterIPOp("Mix", Mix, MISCOPS, B_FALSE, B_FALSE,
	       "First Img [0..1]", DOUBLE_DT, NULL);
  RegisterIPOp("Normalize", Normalize, MISCOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("Plane", Plane, MISCOPS, B_FALSE, B_FALSE,
	       "U", DOUBLE_DT, "V", DOUBLE_DT, NULL);
#ifndef CMGUI
  RegisterIPOp("Second", Second, MISCOPS, B_FALSE, B_FALSE,
	       "delay (s)", INTEGER_DT, NULL);
#endif /* CMGUI */
  RegisterIPOp("ShowXvgMMStats", ShowXvgMMStats, MISCOPS, B_FALSE, B_FALSE,
	       NULL);
  RegisterIPOp("SeedFill", RegionFillIPOp, MISCOPS, B_FALSE, B_FALSE,
	       "StartX", INTEGER_DT, "StartY", INTEGER_DT,
	       "Fill value", DOUBLE_DT, NULL);
  RegisterIPOp("SetPixelValue", SetPixelValue, MISCOPS, B_FALSE, B_FALSE,
	       "x", INTEGER_DT,
	       "y", INTEGER_DT,
	       "value", DOUBLE_DT,
	       NULL);
  RegisterIPOp("SubtractMean", SubtractMean, MISCOPS, B_FALSE, B_FALSE,
	       NULL);
  RegisterIPOp("Stats", Stats, MISCOPS, B_FALSE, B_FALSE,
	       NULL);
  RegisterIPOp("SubtractMin", SubtractMin, MISCOPS, B_FALSE, B_FALSE,
	       NULL);
#ifndef CMGUI
  RegisterIPOp("SubtractPixVal", SubtractPixVal, MISCOPS, B_FALSE, B_FALSE,
	       "x", INTEGER_DT, "y", INTEGER_DT, NULL);
  RegisterIPOp("System", System, MISCOPS, B_FALSE, B_FALSE,
	       "Filename", STRING_DT, NULL);
#endif /* CMGUI */
  RegisterIPOp("Threshold", Threshold, MISCOPS, B_FALSE, B_FALSE,
	       "Lower Threshold", DOUBLE_DT, "Upper Threshold", DOUBLE_DT,
	       "Lower Repl. Val", DOUBLE_DT, "Upper Repl. Val", DOUBLE_DT,
	       NULL);
  RegisterIPOp("ZeroCenter", ZeroCenter, MISCOPS, B_FALSE, B_FALSE, NULL);
  RegisterIPOp("ZeroEdges", ZeroEdges, MISCOPS, B_FALSE, B_FALSE,
	       "Width", INTEGER_DT, NULL);

  /* Video recording Ops */
  RegisterIPOp("MeshVideoLoop", MeshVideoLoop, VIDEOOPS, B_FALSE, B_FALSE,
	       "InFilename", STRING_DT, "Frames/cycle", INTEGER_DT,
	       "Scale (0.0 : NoStrain)", DOUBLE_DT,
	       "Bckgrd files", STRING_DT, NULL);
  RegisterIPOp("RecordFrames", RecordFrames, VIDEOOPS, B_FALSE, B_FALSE,
	       "nframes", INTEGER_DT, NULL);
}
  


