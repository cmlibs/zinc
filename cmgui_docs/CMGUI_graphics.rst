graphics in CMGUI
=================

.. _graphics window: http://www.cmiss.org/cmgui/wiki/UsingCmguiTheGraphicsWindow
.. _scene editor: http://www.cmiss.org/cmgui/wiki/UsingCmguiTheSceneEditorWindow
.. _glyphs: http://www.cmiss.org/cmgui/wiki/VisualizingFieldsAtPointsUsingGlyphs
.. _material editor window: http://www.cmiss.org/cmgui/wiki/UsingCmguiMaterialEditor
.. _spectrum editor window: http://www.cmiss.org/cmgui/wiki/UsingCmguiTheSpectrumEditorWindow
.. _example a7: http://cmiss.bioeng.auckland.ac.nz/development/examples/a/a7/index.html
.. _example ao: http://cmiss.bioeng.auckland.ac.nz/development/examples/a/ao/index.html
.. _curl: http://www.math.umn.edu/~nykamp/m2374/readings/divcurl/

General description of graphics
-------------------------------

graphics are the building blocks used to create any visualization displayed in the CMGUI `graphics window`_. They are created, edited, re-ordered and deleted from within the `scene editor`_, or via the command line. Most graphics have the following settings in common:

* Name:  This allows you to set the name of the graphic.

* Coordinate System: This setting can be used to render the graphic according to a range of different coordinate systems. This is useful for creating "static" overlays of data on visualizations.

* Coordinate field: This setting has a drop-down menu showing a list of fields that can be selected as the coordinate field for the selected graphic.

* Tessellation: Tessellation settings are used to set the level of detail of an object. This replaces the discretization settings from old versions of cmgui.

NOTE:
  The only graphics which do not have a tessellation setting are *node points*, *data points*, and *point*.

* Select mode: This drop-down menu allows you to select different selection behaviours for the graphic:

  * select_on - The default setting; graphics are able to be selected and selected items are highlighted by rendering in the *default_selected* material.
  * no_select - No selection or highlighting of the graphic.
  * draw_selected - only selected items are drawn.
  * draw_unselected - only unselected items are drawn.

* Material: This drop-down menu allows you to select which material should be used to render the graphic. Materials are defined and edited in the `material editor window`_.

* Data: This setting has a drop-down menu, allowing you to select which field will be mapped on to the graphic. This enables you to colour the graphic according to the values of some field, for example. The check box also activates the *spectrum* drop-down menu.

* Spectrum: This drop-down menu is used to select which spectrum is to be used to colour the graphical element according to the field selected in the *data* setting. Spectra are edited in the `spectrum editor window`_.

* Selected material: This drop-down menu allows you to set which material will be used to render parts of the graphic which are selected.

The eight types of graphics
---------------------------

* **node_points**
  
  Node points are used to visualize nodes. You can use `glyphs`_ to represent node points. There are a range of built-in glyphs in CMGUI, and it is possible to create custom glyphs as well. *Node points* graphics have the following settings in addition to the common ones listed above:
  
  * Glyph: this drop-down menu allows you to choose the glyph that will be rendered at the node points.
  * Offset: this box allows you to offset the origin of the glyph in order to alter its placement with respect to the node point.
  * Base glyph size: This box allows you to enter a size (x,y,z) for the glyph in the same scale as the coordinate system for the region.
  * Orientation/Scale: This check box enables a drop-down menu that allows selection of the field that the glyphs will be oriented and scaled according to.
  * Scale factors:  This box allows you to enter how each dimension scales according to the orientation/scale field value.
  * Variable scale: This check box enables a drop-down menu that allows selection of the field that acts as the variable scale field. For more information on this and other of the *node points* options, see the document on working with `glyphs`_.
  * Label: use this drop-down menu allows you to add field-value labels to the glyphs.
  


* **data_points**
  
  Data points are used to visualize data points. Like node points, they can be represented using `glyphs`_. They have the same settings as *node points*.


* **lines**
  
  Lines are used to visualize 1D elements, or the edges of 2D or 3D elements. They are simple, unshaded lines that have a fixed, specified width. They have the following specific settings:

  * Exterior: This check box will automatically only render lines on exterior surfaces of a mesh.
  * Face: This check box enables a drop-down menu that allows you to choose which faces are drawn according to xi values. You can select xi=0 or 1 for each of the three xi-directions.
  * Width: this allows you to specify the width of the lines in pixels. This is a constant width that does not scale according to the zoom level.

* **cylinders**
  
  Cylinders are used to visualize the same things as lines. They are shaded cylinders of a specified radius. They have the following specific settings:

  * Exterior: This check box will automatically only render lines on exterior surfaces of a mesh.
  * Face: This check box enables a drop-down menu that allows you to choose which faces are drawn according to xi values. You can select xi=0 or 1 for each of the three xi-axes.
  * Constant radius: This allows you to set the radius of the cylinders, in the units of the coordinate system.
  * Scalar radius: This check box will activate a drop-down menu allowing you to select which field will be used to scale the radius of the cylinders. It will also activate a text box in which you can enter the scale factor, or how the scale field will scale the radius.
  * Circle discretization: This sets the number of sides used to render the cylinders in the 3D window.
  * Texture coordinates: This drop-down menu allows you to select which field will be used to position any textures applied by the material setting.

* **surfaces**

  Surfaces are used to visualize 2D elements or the faces of 3D elements. They are shaded surfaces of zero thickness that are automatically shaped according to the nodes defining the element they represent. Their level of detail is specified per surface by choosing a *tessellation* object. They have the following specific settings:
  
  * Exterior: This check box will automatically only render surfaces on exterior surfaces of a mesh.
  * Face: This check box enables a drop-down menu that allows you to choose which faces are drawn according to xi values. You can select xi=0 or 1 for each of the three xi-axes.
  * Render type: This drop down menu allows you to select shaded (default) or wireframe rendering of surfaces. Wireframe rendering renders the surfaces as grids of shaded lines, with the grid detail determined by the *tessellation* setting.
  * Texture coordinates: This drop-down menu allows you to select which field will be used to position any textures applied by the material setting.


* **iso_surfaces**
  
  Iso-surfaces are used to represent a surface that connects all points that share some common value. For example, in `example a7`_ an iso-surface is used to represent a surface at which every point has a temperature of 100 degrees C. They have the following specific settings:
  
  * Use element type: This drop down menu allows you to select which type of element will have surfaces rendered on it. Type *use_elements* is the default. The types *use_faces* and *use_lines* will render element points only on those components of elements. If faces or lines are chosen, the following options are activated:

    * Exterior: This check box will automatically only render iso-surfaces on exterior surfaces of a mesh. 
    * Face: This check box enables a drop-down menu that allows you to choose on which faces iso-surfaces are drawn, according to xi values. You can select xi=0 or 1 for each of the three xi-axes.
    
  It is worth noting that if you select *use_surfaces* then the equivalent of iso-surfaces becomes iso-lines. If you select *use_lines* then you will not get any visual representation.
  
  * Iso-scalar: This drop down menu allows you to select the field that the iso-surface will be rendered according to the values of.
  * Iso-values: This settings box contains the following settings:
  
    * List: This radio button activates a text box that allows you to enter a value at which to draw the iso-surface.
    * Sequence: This radio button activates three text boxes that allow you to enter a sequence of evenly spaced values to draw iso-surfaces at. The *Number* box allows you to enter the number of iso-surfaces you want. The *First* and *Last* boxes allow you to enter the starting and ending values of the iso-surfaces. The sequence will automatically space the number of surfaces between these two values.
    
  * Render type: This drop down menu allows you to select shaded (default) or wireframe rendering of surfaces. Wireframe rendering renders the surfaces as grids of shaded lines, with the grid detail determined by the chosen *tessellation* object.
  * Texture coordinates: This drop-down menu allows you to select which field will be used to position any textures applied by the material setting.

* **element_points**
  
  Element points are used to visualize the discretized points within an element. Elements may be 1, 2 or 3 dimensional, in which case the element points are spaced along the line, across the surface, or throughout the volume according to the chosen *tessellation* object . They have the following specific settings:
  
  * Use element type: This drop down menu allows you to select which type of element will have element points rendered on/in it. Type *use_elements* is the default, and renders element points throughout 3D elements. The types *use_faces* and *use_lines* will render element points only on those components of elements. If faces or lines are chosen, the following options are activated:
  
    * Exterior: This check box will automatically only render element points on exterior surfaces of a mesh.
    * Face: This check box enables a drop-down menu that allows you to choose on which faces element points are drawn according to xi values. You can select xi=0 or 1 for each of the three xi-axes.
    
  * Xi discretization mode: this drop down menu allows you to select the method by which element points are distributed across the element.
  
  * 

* **streamlines**
  
  Streamlines are a special graphic for visualizing *vector* fields - for example, a fluid flow solution. They can be used to visualize 3, 6 or 9 component vector fields within a 3 dimensional element. In `example ao`_, streamlines are used to show the fibre and sheet directions in the heart. Streamlines will align along their length according to the first vector of a vector field, and across their "width" (eg the width of the *ribbon* or *rectangle* streamline types) to the second vector. For single vector (3 component) vector fields, the width of the streamlines will align to the curl_ of the vector.
  
  Note that streamlines can be quite expensive to compute; changes to streamline settings in the `scene editor`_ can take several seconds to appear in the 3D window, especially for complex scenes.

  Streamlines have the following specific settings:
  
  * Streamline type: This drop-down box allows you to select the shape of the streamlines; that is, the shape outline that is extruded along the length of the streamline. *Line* and *Cylinder* can be used to visualize streamlines without showing orientation (curl). *Ellipse*, *rectangle* and *ribbon* types will enable visualization of the direction of the vector orthogonal to the streamline direction.
  
  * Length: Enter a value into this box to set the length of the streamline/s.
  
  * Width: Enter a value into this box to set the width of the streamline/s.
  
  * Stream vector: This drop-down box allows you to select the vector that is being visualized by the streamlines.
  
  * Seed element: This setting has a check box which when ticked allows you to specify a single element to seed a streamline from.
  
  * Xi discretization mode: This drop-down box allows you to set the point in xi-space from which streamlines are seeded. The setting of *exact_xi* for example will always seed the streamline at the exact centre of the element's xi-space.
  
  * Reverse: Checking this box reverses the streamline.
  
  * Seed element: Checking this box allows you to select the single element number from which the streamline will be seeded.
  
  * Xi: Entering three comma-separated values (between 0 and 1) allows you to set the xi location within elements from which streamlines will be seeded.

* **point**

  Point graphics are used to add a single glyph to the scene. This is the graphical setting that is used to replace the old axis creation, for example.
