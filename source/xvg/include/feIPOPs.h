/***********************************************************************
*
*  Name:          feIPOPs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef feIPOPs_h
#define feIPOPs_h
extern void BuildInterpolatedImage(double *inpixels, double *outpixels,
				   int height, int width, void *ParmPtrs[]);
extern void CaptureOverlay(double *inpixels, double *outpixels,
		    int height, int width, void *ParmPtrs[]);
extern void CompDispField(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[]);
extern void ComputeMarkerPath(double *inpixels, double *outpixels,
			      int height, int width, void *ParmPtrs[]);
extern void CompStrainField(double *inpixels, double *outpixels,
			    int height, int width, void *ParmPtrs[]);
extern void DrawFEMarkers(swidget DrawingArea);
extern void FECompute(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[]);
#ifdef FE_COMPUTE_DISPLACED_NODES_SPECKLE
extern void FEComputeDisplacedNodes(double *inpixels, double *outpixels,
				    int height, int width, void *ParmPtrs[]);
#endif
extern void FEComputeDispNodesXCorr(double *inpixels, double *outpixels,
				    int height, int width, void *ParmPtrs[]);
extern void FEMeshOverlay(swidget DrawingArea);
extern void FEStrain(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void GenerateSampleFile(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void LoadFEMeshMenu(void);
extern void LoadMesh(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void MeshVideoLoop(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[]);
extern void PickMeshCoordinate(int x, int y);
extern void RegionMaskCreation(double *inpixels, double *outpixels,
			       int height, int width, void *ParmPtrs[]);
extern void System(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[]);
extern void VAFCompute(double *inpixels, double *outpixels, int height,
		       int width, void *ParmPtrs[]);
extern void ReadCMISSFieldSamplesFile(double *inpixels, double *outpixels,
				      int height, int width,
				      void *ParmPtrs[]);
extern void WriteCMISSFieldSamplesFile(double *inpixels, double *outpixels,
				       int height, int width,
				       void *ParmPtrs[]);
static boolean_t WriteCMISSForcesFile(double *InitialForces,
				      double *InitialForcesScaledOnly,
				      double *forces, double *forceOffsets,
				      int cx, int cy, char *name);
extern void WriteCMISSNodesFile(double *inpixels, double *outpixels,
				int height, int width, void *ParmPtrs[]);
extern void WriteCMISSPointsFile(double *inpixels, double *outpixels,
				 int height, int width, void *ParmPtrs[]);
extern void Xvg_InitFE(void);
#endif

