CMGUI Graphics Window
=====================

.. _scene editor: http://www.cmiss.org/cmgui/wiki/UsingCMGUITheSceneEditorWindow

The graphics window is where all visualizations set up in the `scene editor`_ window are displayed. It also has tools which allow some interactive manipulation of the data being visualized. The window consists of a control panel on the left hand side, and the display area on the left. At the top of the control panel area are a selection of general controls under the "Options" label: view all, save as, and perspective. The view all button will zoom out the viewing area so that all the graphics are visible. The save as option provides the ability to save the viewing area as a raster graphic, such as a png or jpg file. The perspective check box switches convergence on or off in the 3D display.

Immediately underneath the options section are controls for selecting how the 3D display is configured.  The *Layout* drop down list contains a list of display layouts, such as orthogonal or pseudo-3D views.  If an applicable layout is selected, the *up* and *front* controls will become available in order to change the viewpoint.  Many of these layouts contain multiple *scene viewers*, which may be useful in specific situations.  The default layout *simple* consists of a single 3D scene viewer.

.. figure:: video here
   :figwidth: image
   :align: center

 **Figure 1:** Selecting a variety of layouts in the CMGUI graphics window.

Below the layout section is a button labelled "Time Editor".  This opens the timeline controls (much like the controls in a media player) that allow you to play animations if they have been set up.

Below this are the 5 "Tools" buttons.  These offer different ways of interacting with the 3D display.

| 

The graphics window tools
-------------------------

* **Transformation mode**

  This is the default mode, and is used for simple viewing of the graphical representations you are working on.  In this mode the objects in the 3D window can be rotated, translated and zoomed using the mouse.  Holding the left mouse button within the 3D view and moving it around will rotate, or tumble the view.  Holding down the right mouse button and moving the mouse up and down will zoom the view in and out, and holding the middle button will allow you to translate the view around along two axes.  It is useful to spend some time getting used to the way these manipulations work.

  In CMGUI, the rotate function works slightly differently from how similar view manipulations work in most software.  This may not be immediately obvious, as the function does not "feel" particularly different in use; nevertheless there are some useful features of CMGUI's particular technique for rotating the view.

  Essentially, where you initially click in the view is the "handle" that you then move around by moving the mouse.  In CMGUI this handle is different to most applications, in that it is like "grabbing" a point on a sphere that bounds the object in the 3D window.  This allows manipulations using the rotate function that are not possible in most 3D views, such as rotating the object around an arbitrary axis, or rotating it in a circular fashion around the centre of the view.  These abilities can be useful when looking at data that has aligned features.

  The four other tools available are used for the selection and limited editing of the type of item they refer to.  Selected items are able to be targeted by commands input to the command line, or edited from within the graphics window.
  
  When any of the following four tools is selected, holding down the ``Ctrl`` key will temporarily switch you back into transformation mode in order to manipulate the view.

* **Node Tool**
  
  The node tool allows the selection and editing of individual nodes from within the graphics window.  Selected nodes will turn red by default - the selected colour of a node is editable via the `scene editor`_ window.  There are a range of other options to allow the editing, (moving nodes within the scene viewer) deletion, or creation of nodes.  It is also possible to create 1D, 2D or even 3D (lines, surfaces and volumes) elements using the node tool; this functionality is somewhat experimental and is of limited use in most cases.
  
  NOTE: It is somewhat easier to edit nodes (or indeed any of the other editable items) when they are represented by an easily clickable glyph such as a sphere or cube, rather than a point.

* **Data Tool**
  
  The data tool allows you to select data points and edit them in the same ways that nodes are editable, with the exception of element creation/destruction which is only available in the node tool.

* **Element Tool**
  
  The element tool allows you to select and destroy elements.  You can also set up filters that allow only the selection of line, face or volume elements.  It is worth noting that volume elements have no indication of their selection unless they contain element points, which will turn red when the volume element they exist in is selected.

* **Element Point Tool**
  
  The element point tool allows the selection of element points within the scene viewer/s.

| 

All of the tools that allow selection are useful for creating *groups* via the command line.  You can add selected items to a group using the commands