//  -*- C++ -*-
// Copyright 1996-1997, SensAble Technologies, Inc.
//  file: gstScene.h

/*
_______________________________________________________
_____________ SensAble Technologies, Inc. _____________
_______________________________________________________
__
__	Rev:			1.0
__
__  Class:			gstScene
__
__	Description:	Manages dynamic/haptic simulation
__					and owns haptic process.  Also, 
__					arbitrates communication to and
__					from the haptic process.
__
__	Authors:		Chris Tarr	
__
__	Revision:		W. A. Aviles	Rev 1.0	
_______________________________________________________
_______________________________________________________
*/

#ifndef GST_SCENE
#define GST_SCENE

extern long sginap(long);
#include <gstBasic.h>
#include <gstTransform.h>

// Scene class.  This class manages the scene database, dynamic/haptic
// simulation and owns the haptic process.  In addition, the scene class 
// arbitrates communication to and from the haptic process.
class  gstScene {

public:

	enum {
		SMOOTHING_ON,
		SMOOTHING_OFF
	};

	// Constructor.
	gstScene();

	// Destructor.
	~gstScene();

	// FOR_GHOST_INTERNAL:
        // NOT CURRENTLY SUPPORTED.
        // Scene lock.
	void			lock() {
		if (!doneServoLoop) {
			needLock = TRUE;
			while (needLock)
			  Sleep(0);
		}
	}

	// FOR_GHOST_INTERNAL:
        // NOT CURRENTLY SUPPORTED.
	// Scene unlock.  
	void			unlock() {
		locked = FALSE;
	}

	// Set root node of haptic scene graph.
	void			setRoot(gstTransform *newRoot) {
		if (rootNode != NULL)
			rootNode->removeFromSceneGraph();
		rootNode = newRoot;
		if (newRoot != NULL)
			newRoot->putInSceneGraph();
	}

	// Get root node of haptic scene graph.
	gstTransform			*getRoot() {
		return rootNode;
	}

	// FOR_GHOST_INTERNAL:
	// Turns ON forces of all PHANToMs in the scene graph.
	void				turnForcesOn() {
		forcesOn = TRUE;
		firstLoop = TRUE;
	}

	// FOR_GHOST_INTERNAL:
	// Turns OFF forces of all PHANToMs in the scene graph.
	void				turnForcesOff() {
		forcesOn = FALSE;
		firstLoop = TRUE;
	}

	// Set smoothing level for scene.  Smoothing attempts to 
	// remove high-frequency variations in gstPolyMesh geometries.
	// Currently only 0 (OFF) and 1 (ON) are supported.
	void				setSmoothing(int newLevel) {
		smoothingLevel = newLevel;
	}

	// Get smoothing level of the scene.
	int					getSmoothing() const {
		return smoothingLevel;
	}

	// Has simulation end/continue from occurence of a device
	// fault from any gstPHANToM in scene graph.  If flag is TRUE
	// then simulation will exit if any gstPHANToM in the scene
	// graph has a device fault.  If flag is FALSE, then device
	// faults from any gstPHANToM in the scene graph will cause
	// all forces to be turned off, but simulation will be
	// maintained.  As soon as the all gstPHANToMs in the scene
	// graph have no device faults, forces will be restarted.
	// If a device fault occurs will trying to start forces,
	// simulation will exit regardless of the value of flag.
	// Note: Device faults occur when the PHANToM device is
	// not hooked up properly to the computer, or when the remote
	// switch is not activated.
void				setQuitOnDevFault(gstBoolean flag) {
		quitOnDevFault = flag;
	}

	// Returns the current value of quitOnDevFault.
	gstBoolean			getQuitOnDevFault() const {
		return quitOnDevFault;
	}

	// Activate all objects in the scene graph needing 
	// a graphics update since the last call to
	// updateGraphics. This causes all such objects
    // to call their appropriate graphics callbacks
	// with the most current state information.
	void			updateGraphics() {
		preparingGraphics = FALSE;
		if (!doneServoLoop)
			preparingGraphics = TRUE;
		while (preparingGraphics) {Sleep(0);};
		gstTransform::staticUpdateGraphics();
	}

	// Activate all objects in the scene graph that 
	// have had events occur since the last call to
	// updateEvents. This causes all such objects
    // to call their appropriate event callbacks for
	// each event that has occured since the last
	// call to updateEvents.
	void			updateEvents() {
		preparingEvents = FALSE;
		if (!doneServoLoop)
			preparingEvents = TRUE;
		while (preparingEvents) {Sleep(0);};

		gstTransform::staticUpdateEvents();
	}
         
	// Start the haptic simulation as separate process.
	// Control is returned immediately.
	int				startServoLoop();

	// Stop the haptic simulation.
	void			stopServoLoop();


	// FOR_GHOST_INTERNAL:
	// Returns whether the haptics process is currently in
	//  the servoLoop.
	int				getDoneOneLoop() {return doneOneLoop;}

	// Returns TRUE if haptics process has finished (i.e.
        // if the servo loop is not running).
	int				getDoneServoLoop() const { return doneServoLoop;}

	  // Turns safety on and off.  If safety is off, the servo loop is
	  // allowed to take as much CPU time as necessary to finish.
	  // This may result in instability or crashes.  The default is TRUE (on).
	void setSafety(gstBoolean _s) {
		scene_safety = _s;
	}
  
	// Returns TRUE if safety limits are on.
	gstBoolean getSafety() {
		return scene_safety;
	}

gstInternal public:

	// FOR_GHOST_INTERNAL:
	// Used by system or for creating sub-classes only.
	// Servo loop body.
	int				servoLoop();

	// FOR_GHOST_INTERNAL:
 	// Used by system or for creating sub-classes only.
        // Static method to initialize class data.
	static int initClass();

	// FOR_GHOST_INTERNAL:
	// Used by system or for creating sub-classes only.
	void			setDoneOneLoop(gstBoolean newVal) {doneOneLoop = newVal;}

protected:

	gstTransform			*rootNode;
	int						preparingGraphics, preparingEvents;
	int						doneServoLoop, doneOneLoop;
	int						dynamicF;
	double					dynamicFrictionAvg, staticFrictionAvg, surfaceDampingAvg;
	double					Kfriction;
	int						firstLoop, forcesOn;
	gstBoolean				needLock, locked;
	gstBoolean				quitOnDevFault;
	int						smoothingLevel;
  gstBoolean scene_safety;

};



#endif // GST_SCENE
