Visualizing element fields using iso-surfaces
=============================================

.. _scene editor: http://www.cmiss.org/cmgui/wiki/UsingCMGUITheSceneEditorWindow
.. _graphical settings: http://www.cmiss.org/cmgui/wiki/CMGUIGraphicalSettings
.. _graphics window: http://www.cmiss.org/cmgui/wiki/UsingCMGUITheGraphicsWindow
.. _example a7: http://cmiss.bioeng.auckland.ac.nz/development/examples/a/a7/index.html
.. _example a2: http://cmiss.bioeng.auckland.ac.nz/development/examples/a/a2/index.html
.. _surfaces: http://www.cmiss.org/cmgui/wiki/VisualizingElementFieldsUsingSurfaces

*Iso-surfaces* are graphical representations used to visualize 3D or 2D objects that connect every point where the value of a certain field is the same.  This idea is analogous to the contour lines on a map, where a line connects every point of a certain height.  In 3D this means that a surface connects every point of a certain value.  An example of this is in `example a7`_ where an iso-surface is used to show the surface where every point is at 100 degrees Celsius.  For a simpler example, `example a2`_ shows the creation of an iso-surface at x=0.5 within the simple cube mesh.

| 

.. figure::
   :figwidth: image
   :align: center

   **Figure 1: Iso-surfaces:** This video shows some of the basics of working with iso-surfaces in cmgui, using the mesh from `example a7`_.

| 

As with surfaces_, the detail level of iso-surfaces is determined by the *element discretization* setting in the *general settings* area of the `scene editor`_ (Figure 2).  Iso-surfaces are drawn to connect points on the edges of the sub-element divisions where the iso-scalar matches the chosen value.  With an *element discretization* setting of 1*1*1, an iso-surface will only connect points on the line edges of the element.

| 

.. figure:: discretization_iso_surfaces.png
   :figwidth: image
   :align: center

   **Figure 2: How discretization affects iso-surface detail:**  The *element discretization* setting in the *general settings* section of the scene editor sets the number of divisions in each xi direction within each element.  Here, an iso-surface is used to connect every point within the element cube where "temperature" is 20.  The "temperature" field is shown on the surface of the element cube using a spectrum in panel A.  In B, the iso-surface is shown in wireframe (green) and a single face of the cube is divided up according to the *element discretization* setting.  You can see that the iso-surface links the discretization divisions at points where the iso-scalar is at the chosen value.  These intersections are indicated by yellow circles. Panel C shows how different *element discretization* settings affect this iso-surface; settings of 2, 3, 4, and 8 are shown.



