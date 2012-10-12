//  -*- C++ -*-
// Copyright 1996-1997, SensAble Technologies, Inc.
// file: gstTransform.h

/*
_______________________________________________________
_____________ SensAble Technologies, Inc. _____________
_______________________________________________________
__
__	Rev:			1.0
__
__  Class:			gstTransform
__
__	Description:	Abstract node class that adds 3d
__					transformations and callbacks to
__					nodes.
__
__	Authors:		Chris Tarr	
__
__	Revision:		W. A. Aviles	Rev 1.0	
__
_______________________________________________________
_______________________________________________________
*/

#ifndef GST_TRANSFORM
#define GST_TRANSFORM	

#include <gstTransformMatrix.h>

// Graphics callback data for gstTransform.  When
// prepareForGraphicsUpdate is called, graphics
// information is copied to this structure, which
// is then passed to the graphics callback in
// updateGraphics().  All subclasses of
// gstTransform that need to add callback data
// must define a new structure that has a
// superset of this structure's data and call
// its parents prepareForGraphicsUpdate and
// updateGraphics methods after their own are called.
typedef struct _gstTransformCBData {
	gstTransformMatrix	transform;
	gstTransformMatrix	cumulativeTransform;
} gstTransformGraphicsCBData;

#include <gstEventStack.h>
#include <gstNode.h>

// Abstract node class that adds 3D transformations and callbacks 
// to nodes.
class gstTransform:public gstNode
{

public:
	// Constructor.
	gstTransform();

	// Constructor.
	gstTransform(const gstTransform *);

	// Constructor.
	gstTransform(const gstTransform &);

	// Destructor.
	virtual ~gstTransform();

	friend class gstRotateManipulator;

	// GHOST_IGNORE_IN_DOC
	// Get type of this class.  No instance needed.
	static gstType		getClassTypeId() { return gstTransformClassTypeId; }

	// GHOST_IGNORE_IN_DOC
	// Get type of this instance.
	virtual gstType		getTypeId() const { return gstTransformClassTypeId; }

	// GHOST_IGNORE_IN_DOC
	// Returns TRUE if this class is same or derived class of type.
	virtual gstBoolean	isOfType(gstType type) const{
		if (type == gstTransformClassTypeId) return TRUE;
		else // Check if type is parent class.
			return (gstNode::staticIsOfType(type));
	}

	// GHOST_IGNORE_IN_DOC
	// Returns TRUE if subclass is of type.
	static gstBoolean	staticIsOfType(gstType type) {
		if (type == gstTransformClassTypeId) return TRUE;
		else return (gstNode::staticIsOfType(type));
	}

	// Returns parent of node in graph.  
        // Returns NULL if the node has no parent or is the root of the scene graph.
	gstTransform			   *getParent() const { return parent; }


	//   Set graphics callback.  "start_address" points to a user callback function that is
	//   called when gstScene::updateGraphics is called by the
	//   application and this instance of gstTransform or its
	//   subclasses have new graphics information.  Graphics
	//   information is passed as the second parameter
	//   "callbackData" and should be cast to the type  
	//   (CLASSNAMEgraphicsCBData *) for the correct class of this instance.
	void				setGraphicsCallback( void ( *start_address )( gstTransform *thisPtr, void *callbackData, void *userData ),
		void *param);

	//   Set event callback.  "start_address" points to a user callback function that is
	//   called when gstScene::updateEvents is called by the
	//   application and this instance of gstTransform or its
	//   subclasses have new event information.  Event
	//   information is passed as the second parameter
	//   "callbackData" and should be cast to type (gstEvent *).
	//   The fields of this structure are interpreted differently
	//   by each class and you should consult the GHOST Programming Guide
	//   for an list of nodes and their interpretation of these fields for
	//   various events.
	void				setEventCallback( void( *start_address )( gstTransform *thisPtr, void *callbackData, void *userData ),
		void *param);

	// Get graphics callback user data.
	void				*getGraphicsCBUserData() const {
		return graphicsCBUserData;
	}


	//  Grouping nodes call the same function for each of their
	//  children propagating the effect down the scene graph.  Any
	//  gstShape nodes with this method called use the input flag
	//  to indicate if they are touchable by any gstPHANToM node
	//  in the scene graph.  If FALSE, then the gstShape node is
	//  transparent.  Otherwise, the gstShape node is able to 
	//  be contacted by any gstPHANToM node in the scene graph.
	virtual void		touchableByPHANToM(gstBoolean) {}

	//  Sets homogenous transformation matrix.
	//  Note: Any call to setTransformMatrix resets all scale
	//  rotate, and translate values set previously.  Conversly,
	//  any call to setRotate, setScale or setTranslate resets 
	//  the previous call to setTransform.  Setting a matrix 
	//  explicitly using this method or any of the array accessors
	//  to set a specific entry of the 4x4 matrix will cause 
	//  this transform matrix to become "User Defined".  A
	//  user defined matrix ceases to have the CSRTC' composite
	//  form described in the GHOST Programming Guide.  Instead,
	//  no composite form is assumed and some operations may run
	//  slower with the "User Defined" matrix since some 
	//  optimizations are not performed.
	void 				setTransformMatrix(const gstTransformMatrix &matrix);

	// Get homogenous transformation matrix.
	void				getTransformMatrix(gstTransformMatrix &matrixArg);

	// Get homogenous transformation matrix.
	gstTransformMatrix	getTransformMatrix() {
		gstTransformMatrix M;
		getTransformMatrix(M);
		return M;
	}
  
	// Get cumulative transformation matrix.
	void				getCumulativeTransformMatrix(gstTransformMatrix &matrixArg);

	// Get cumulative transformation matrix.
	gstTransformMatrix  getCumulativeTransformMatrix() {
		gstTransformMatrix cM;
		getCumulativeTransformMatrix(cM);
		return cM;
	}

	// Get scale orientation matrix.
	void				getScaleOrientationMatrix(gstTransformMatrix &matrixArg);
	

	// Get scale orientation matrix.
	gstTransformMatrix  getScaleOrientationMatrix() {
		gstTransformMatrix cM;
		getScaleOrientationMatrix(cM);
		return cM;
	}

	// Get rotation matrix.
	void				getRotationMatrix(gstTransformMatrix &matrixArg);

	// Get rotation matrix.
	gstTransformMatrix  getRotationMatrix() {
		gstTransformMatrix cM;
		getRotationMatrix(cM);
		return cM;
	}

	// Get equivilant rotation of current rotation matrix (orientation) based 
	// on successive rotations around x,y,z axes.  Angles are in radians
	// and use right hand rule.
	gstPoint getRotationAngles() const {
		gstPoint axes;
		objTransf.getRotationAngles(axes);
		return axes;
	}

	// Get equivilant rotation of current rotation matrix (orientation) based 
	// on successive rotations around x,y,z axes.  Angles are in radians
	// and use right hand rule.
	void getRotationAngles(gstPoint &axes) const {
		objTransf.getRotationAngles(axes);
	}


	// Overwrite previous center position of node with new center position.
	// Not supported for gstShape classes.
	virtual void				setCenter(const gstPoint &newCenter);	

	// Get x,y,z coordinates of center.  Not supported for gstShape classes.
	virtual void				getCenter(gstPoint &centerArg) const;

	// Get x,y,z coordinates of center.  Not supported for gstShape classes.
	virtual gstPoint	getCenter() {
		gstPoint centerArg;
		getCenter(centerArg);
		return centerArg;
	}

	// Get position in local coordinate reference frame.
	virtual void	getPosition(gstPoint &pos) {
		if (objTransf.getUserDefined())
			pos.init(objTransf.get(3,0), objTransf.get(3,1), objTransf.get(3,2));
		else
			objTransf.getTranslation(pos);

	}

	// Get position in local coordinate reference frame.
	virtual gstPoint	getPosition() {
		gstPoint pos;
		getPosition(pos);
		return pos;
	}

	// Get x,y,z translation in world coordinates.
	virtual void		getPosition_WC(gstPoint &pos) {
		getTranslation_WC(pos);
	}
 
	// Get x,y,z translation in world coordinates.
	virtual gstPoint	getPosition_WC() {
		gstPoint pos;
		getPosition_WC(pos);
		return pos;
	}

	// Overwrite previous translation with new translation.
	virtual void		setPosition(const gstPoint &newPos) {
	  setTranslate(newPos);
	}

	// Overwrite previous translation with new translation given as
	// a position in world reference frame coordinates.
	virtual void		setPosition_WC(const gstPoint &newPos_WC) {
		if (parent != NULL)
			setTranslate(parent->fromWorld(newPos_WC));
		else
			setTranslate(newPos_WC);
	}

	// Overwrite previous translation with new translation.
	virtual void		setPosition(double x,double y,double z) {
	  setTranslate(gstPoint(x,y,z));
	}

	// Overwrite previous translation with new translation.
	virtual void		setTranslate(double x, double y, double z) {
	  setTranslate(gstPoint(x,y,z));
	}
  
	// Accumulate translation with previous translation of node.
  virtual void translate(double x, double y, double z) {
    translate(gstPoint(x,y,z));
  }

	// Accumulate translation with previous translation of node.
	virtual void				translate (const gstPoint &translation);

	// Set translation.
	virtual void				setTranslate (const gstPoint &translation);

	// Get translation in local coordinate reference frame.
	virtual void				getTranslation(gstPoint &translationValue) const;

	// FOR_GHOST_INTERNAL
	// Get translation in world coordinate reference frame.
	virtual void				getWorldTranslation(gstPoint &translationValue) {
		getTranslation_WC(translationValue);
	}

	// Get translation in world coordinate reference frame.
	virtual void				getTranslation_WC(gstPoint &translationValue);

	// Accumulate rotation with previous rotation of node using vector/angle method [radians].
	virtual void				rotate(const gstVector &axis, double rad);

	// Overwrite previous rotation of node using vector/angle method [radians].
	virtual void				setRotate(const gstVector &axis, double rad);

	// FOR_GHOST_INTERNAL
	// Not currently supported.
	virtual void				getRotation(gstVector &axisArg, double *radArg) const;

	// Accumulate uniform scale with previous scale of node.
	virtual void				scale     (double scale);	

  	// Accumualte scale with previous scale of node.
    virtual void                            scale     (double x, double y, double z) {
		scale(gstPoint(x,y,z));
	}

	// Accumulate scale with previous scale of node.
	virtual void				scale     (const gstPoint &newScale);

	// Overwrite previous scale of node with new scale.
	virtual void				setScale(double x, double y, double z);

	// Overwrite previous scale of node with new uniform scale.
	virtual void				setScale     (double newScale);

	// Overwrite previous scale of node with new scale.
	virtual void				setScale     (const gstPoint &newScale);

	// Get x,y,z scale  factors along scale orientation axis.
	virtual void				getScaleFactor(gstPoint &scaleFactorArg) const ;

	// Get x,y,z scale factors along scale orientation axis.
	virtual gstPoint			getScaleFactor() const {
		gstPoint scaleFactorArg;
		getScaleFactor(scaleFactorArg);
		return scaleFactorArg;
	}

	// Transform point "p", which is in the local coordinate
	// reference frame, to the point in the world coordinate
	// reference frame.
	virtual gstPoint	toWorld(const gstPoint &p);

	// Transform vector "v", which is in the world coordinate
	// reference frame, to the vector in the local coordinate
	// reference frame.
	virtual gstVector	fromWorld(const gstVector &v);

	// Transform point "p", which is in the world coordinate
	// reference frame, to the point in the local coordinate
	// reference frame.
	virtual gstPoint	fromWorld(const gstPoint &p);

	// Transform vector "v", which is in the local coordinate
	// reference frame, to the vector in the world coordinate
	// reference frame.
	virtual gstVector	toWorld(const gstVector &v);

	// Transform point "p", which is in the local coordinate
	// reference frame, to the point in the parent coordinate
	// reference frame.
	virtual gstPoint	toParent(const gstPoint &p){return objTransf.fromLocal(p);}

	// Transform vector "v", which is in the parent coordinate
	// reference frame, to the vector in the local coordinate
	// reference frame.
	virtual gstVector	fromParent(const gstVector &v){return objTransf.toLocal(v);}

	// Transform point "p", which is in the parent coordinate
	// reference frame, to the point in the local coordinate
	// reference frame.
	virtual gstPoint	fromParent(const gstPoint &p){return objTransf.toLocal(p);}

	// Transform vector "v", which is in the local coordinate
	// reference frame, to the vector in the parent coordinate
	// reference frame.
	virtual gstVector	toParent(const gstVector &v){return objTransf.fromLocal(v);}

	// FOR_GHOST_EXTENSION
	//  Get cumulative transformation matrix.
	//  NOTE: result is a non constant reference.  
	virtual gstTransformMatrix
						&getCumulativeTransform();
	
	
gstInternal public:

	// FOR_GHOST_EXTENSION
	// Used by system or for creating sub-classes only.
	// Static method to initialize all static data for this class.
	static int			initClass();

	// FOR_GHOST_EXTENSION
	// Used by system or for creating sub-classes only.
	// Called when object is removed from scene graph.
	virtual void		removeFromSceneGraph();

	// FOR_GHOST_EXTENSION
	// Used by system or for creating sub-classes only.
	// Called when object is put in scene graph.
	virtual void		putInSceneGraph() {
		gstNode::putInSceneGraph();
		addToGraphicsQueue();
	}

	// FOR_GHOST_EXTENSION
	// Used by system or for creating sub-classes only.
	// Invalidate cumulative transformation matrix.
	// When this object or any object above it in the scene changes its local
	// transformMatrix, this node's cumulative transform matrix becomes not valid
	// until it is recomputed based on the new data.  This function invalidates
	// the cumulative transform matrix for this node and its inverse.
	virtual void		invalidateCumTransf();

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Invalidate cumulative transformation matrix and make it untouched.
	// When this object or any object above it in the scene changes its local
	// transformMatrix,  this node's cumulative transform matrix becomes not valid
	// until it is recomputed based on the new data.  This function invalidates
	// the cumulative transform matrix for this node and its inverse.
	virtual void		invalidateCumTransfAndMakeUntouched();

	
	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Set parent.
	void				setParent(gstTransform *newParent);

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Set newDynamicDep as gstDynamic descendent parent of this instance.
	// Indicates whether a node is part of a dynamic system.
	virtual void		setDynamicDependent(gstTransform *newDynamicDep) {
		dynamicDependent = newDynamicDep;
	}

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Make object untouched.  This is called when an object is 
	// transformed using the normal--i.e. non
	// dynamic--transformations.  It should turn the
	// forces off for all geometry objects under and
	// including this node until the PHANToM leaves
	// the object (if the PHANToM is inside the object).
	virtual void		makeUntouched() {}

	// Get dynamic dependent.
	gstTransform		*getDynamicDependent() const {
		return dynamicDependent; }
  
	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Returns TRUE if object is in graphics queue.
	gstBoolean			getInGraphicsQueue() { return inGraphicsQueue; }

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Set up data structures to update graphics for all transforms
	// in the scene that have changed since the last call to 
	// staticPrepareToUpdateGraphics().
	static void			staticPrepareToUpdateGraphics();

	// FOR_GHOST_EXTENSION
	// Used by system or for creating sub-classes only.
	// Set up data structures to update graphics.
	virtual void		prepareToUpdateGraphics();

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Update graphics for all transforms in the scene that have
	// changed since the last call to staticUpdateGraphics().
	static void			staticUpdateGraphics();

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Update graphics.
	void				updateGraphics();

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Set up data structures to update events for all transforms
	// that have changed since the last call to staticPreparetoUpdateEvents().
	static void			staticPrepareToUpdateEvents();

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Set up data structures to update events.
	virtual void		prepareToUpdateEvents();

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Update events for all transforms in the scene graph.
	static void			staticUpdateEvents();

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Update events.
	void				updateEvents();
  
	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Debug method.  Not for use by applications. 
	gstTransformMatrix	getObjectTransformMatrixDEBUG() {
		return objTransf;
	}

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Debug method.  Not for use by applications. 
	gstTransformMatrix	getCumTransformMatrixDEBUG() {
		return cumTransf;
	}

	// FOR_GHOST_INTERNAL
	// Used by system or for creating sub-classes only.
	// Debug method.  Not for use by applications. 
	gstPoint			fromWorldDEBUG(const gstPoint &p);

protected:

	gstTransform		*parent;
	gstTransform		*dynamicDependent;

	// transform matrices
	gstTransformMatrix	objTransf, lastObjTransf;
	gstVector			scaleFactor;
	gstTransformMatrix	cumTransf, lastCumTransf;
	int					cumTransformValid;

	// Event and Graphics Queues data
	gstBoolean			inGraphicsQueue;
	gstBoolean			inEventQueue;
	gstBoolean			changedThisServoLoop;

	static gstTransform	*graphicsQueueHead;
	static gstTransform	*waitingForGraphicsCallbackHead;
	static gstTransform	*eventsQueueHead;
	static gstTransform	*eventsWaitingToBeFiredHead;

	gstTransform		*nextNodeInGraphicsQueue;
	gstTransform		*nextWaitingForGraphicsCallback;
	gstTransform		*nextEventInQueue;
	gstTransform		*nextEventWaitingToBeFired;

	void ( *graphicsCallback )( gstTransform *thisPtr, void *callbackData, void *userData );
	void *graphicsCBUserData;

	// This is used to store graphics data for callback.
	// Memory is allocated in this class's constructor.
	void *graphicsCBData;

	void ( *eventCallback )( gstTransform *thisPtr, void *callbackData, void *userData );
	void *eventCBUserData;

	// Useful methods.
	gstBoolean			addEvent(const gstEvent &newEvent);
	void				addToGraphicsQueue();


private:

	static gstType		gstTransformClassTypeId;

	// NOTE: Other subclasses should not directly touch this.
	gstEventStack		events, eventsWaitingToBeFired;

};
	
#endif
