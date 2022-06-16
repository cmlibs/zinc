﻿The cmgui EX format guide: exnode and exelem files
==================================================

.. contents::

Overview
--------

Spatially-varying "finite element" field definitions are commonly imported into cmgui in two parts:

1. ".exnode" files containing definitions of nodes which are points identified by a unique integer identifier, and for which field parameters are listed.

2. ".exelem" files containing definitions of elements which are n-dimensional coordinate spaces, and the functions giving field values over them, usually by interpolating node field parameters.

This division is arbitrary since they share a common format able to define both nodes and elements, and the fields defined over them. The extensions ".exnode" and "exelem" are merely conventions for the above uses. Any filename extension is acceptable, and full file names with extensions should always be specified in your scripts and command files.

Nodes with literal field values stored for them do not just support element interpolation, but are widely used to represent any point data, and there need not be any associated elements. You will occasionally see files with the ".exdata" extension which are identical to ".exnode" files but read in with a different command ("gfx read data …") so that the objects and associated fields are put in a different set of node objects referred to within cmgui as data rather than nodes, and which are unable to be interpolated in element fields. Note the current limitation of cmgui that each region consists of one list of node objects, one list of elements, and one list of data points.

General Syntax
--------------

Cmgui EX files are human-readable text files which are usually created by export from cm or OpenCMISS. They can also be created by scripts or by hand. Due to their complexity, particularly in defining element interpolation, editing an existing file is usually the easiest way to create new data files by hand.

The format has been around for many years and has several exacting rules which can catch the unwary:

1. Main blocks of the file begin with tokens such as "Group name", "Shape", "#Fields", "Node", "Element" which must be written in full with exact capitalization. Within these blocks are further tokens indicating sub-blocks of data to be read, each with their own rigid rules.

2. The characters used to separate tokens, identifiers, parameters and other data are quite inconsistent and could be white space, commas, colons, full stops (periods), equal signs and so on. In each case the precise character must be used. Whitespace separators generally includes any number of spaces, tabs, end of line and line feed characters. Non-whitespace separator characters can generally be surrounded by whitespace.

3. All array indices start at 1, but scale factor index 0 has the special meaning of using unit scale factor value, 1.0.

4. Node and element identifiers must be positive integers.


Fortunately, cmgui import commands ("gfx read node/element/data") report the line number in the file at which the first errant text is located (do not confuse this with line element numbers). Unfortunately the actual problem may be earlier: if the previous block was incomplete then the first text of the next block, however correct, will be complained about as not matching the tokens and data expected for the previous block.

Regions 
-------

The EX format defines fields, nodes, elements and groups (sets of nodes and elements) within regions. These objects belong to their respective region and are independent of similarly identified objects in other regions.

Cmgui version 2.6 (released: May 2009) introduces a "Region" keyword which specifies the path to the region which any following fields and objects are defined within until the next Region keyword or end of file. The format is as follows::

 Region: /PATH_TO_REGION

Examples::

 ! The root region for this file:
 Region: /
  
 ! Region "joe" within region "bob":
 Region: /bob/joe

The region path must always be written as an absolute path from the root region of the file, i.e. as a series of zero or more region names starting with and separated by forward slash "/" characters. Region names must start with an alphanumeric character and contain only alphanumeric characters, underscores '_' and spaces; spaces are not permitted at the start or end of the name. Colons ':' and dots/periods '.' are permitted but discouraged.

Versions of cmgui prior to 2.6 implicitly read data into the root region, and will report an error for the unrecognised "Region" keyword. The first non-comment keyword in post Cmgui 2.6 EX files should be a Region keyword; to maintain compatibility with older files it may alternatively begin with a Group declaration.

With EX files you are free to put many regions (and groups) in the same file, or to keep each region (or group) in separate files. The choice is up to the user, although it is more flexible to use separate files, since commands for reading EX files into cmgui (e.g. gfx read nodes/elements/data) permit the file's root region to be mapped to any region. For maximum flexibility and to avoid potential merge conflicts, it is recommended that you keep your model data out of the Cmgui root region.

Cmgui example a/region_io gives several example EX files defining fields in regions and sub-groups.

Groups
------

The Group keyword indicates that all following nodes and elements until the next Group or Region keyword are "tagged" as belonging to a group (set) of the specified name::

 Group name: GROUP_NAME

There can be any number of groups in a single region, each potentially sharing some or all of the same nodes and elements from the region they belong to. Groups are entirely contained within their particular region; groups with the same name in different regions are completely independent.

Groups have the same name restrictions as regions. They are not written as a path - just a single name within the enclosing region.

Older versions of Cmgui required nodes, elements and fields to always be defined within a group (within the implied root region), hence the Group keyword was always the first keyword in the file. Since Cmgui 2.6 this is no longer a limitation.

In older EX files it was common to have fields defined after/within a group declaration. However the grouping only ever applies to nodes and element - fields always belong to the region. It is more common to define the fields with nodes and elements under the region with no group, and list only node and element identifiers with no fields under the groups.



Field declaration headers
-------------------------

Whether the EX file is listing nodes or elements, there is broad similarity in the way in which they and their field data is listed. It consists of a header declaring the number and details of fields being defined for the respective object type, followed by the definitions of the objects themselves. An example header::

 #Fields=1
 1) coordinates, coordinate, rectangular cartesian, real, #Components=3
 ... field data specific to nodes or elements ...

The above header declares a field with the name "coordinates". The field is tagged as usage type "coordinate" - this is a hint which tells cmgui that this field is appropriate for use as a coordinate field. Other values for this hint are "anatomical" for special fibre fields, and "field" for all other fields. By default, cmgui will use the first coordinate field in alphabetical name order for the coordinates of graphics. The coordinates declared in this header are embedded in a rectangular Cartesian coordinate system with 3 real-valued components. The rectangular Cartesian coordinate system is assumed if none is specified. The value type real is frequently omitted as it is the default; other types such as integer, string and element_xi are only usable in node fields and integer for grid-based element fields. More complex declarations are given throughout this document. Note that if there is no header or the header has "#Fields=0", then nodes and elements can be defined or listed for adding to a group without defining fields.

Following each line declaring the main details of a field are the details on each field component including the name and the parameter values to be supplied with each node, or the basis functions and parameter mappings to be supplied with each element. These are described later.

There can be only one field of a given name in a region, but it can be defined on nodes, elements and data in that region provided the field is consistently declared in the header, including same value type, numbers of components and component names.

Note that the #Fields keyword in element field headers are additionally preceded by the following keywords and sub-blocks which are described in later examples::

 #Scale factor sets=~
 ...
 #Nodes=~

Comments
--------

Since Cmgui version 2.6 EX files containing comments are now able to be read. Comment lines begin with ! (exclamation mark character) and may only be placed in parts of the file where a Region, Group, Shape, Node, Element, Values or #Field header keyword is permitted. Putting comments anywhere else will result in obscure errors or undefined behaviour!

Comments are useful for adding source details or copyright/license information to your files, or to document parts of the file. Cmgui ignores the comment lines: they will not be rewritten when exporting an EX file. 

Some example comments preceding other keywords::

 ! Copyright (C) 2009 The Author
 Region: /heart
  
 ! This following node is the apex of the heart:
 ! It has 10 versions of all nodal parameters
 Node: 13

Note that errors are reported if you attempt to read EX files with comments into versions of Cmgui prior to 2.6.


Defining nodes and node fields
------------------------------

Example: specifying 3-D coordinates of nodes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Following is an example reading 8 nodes numbered from 1 to 8, with a field "coordinates" giving positions at the corners of a unit cube, adapted from cmgui example a/a2::

  Region: /cube
  Shape. Dimension=0
  #Fields=1
  1) coordinates, coordinate, rectangular cartesian, #Components=3
   x. Value index=1, #Derivatives=0
   y. Value index=2, #Derivatives=0
   z. Value index=3, #Derivatives=0
  Node: 1
   0.0 0.0 0.0
  Node: 2
   1.0 0.0 0.0
  Node: 3
   0.0 1.0 0.0
  Node: 4
   1.0 1.0 0.0
  Node: 5
   0.0 0.0 1.0
  Node: 6
   1.0 0.0 1.0
  Node: 7
   0.0 1.0 1.0
  Node: 8
   1.0 1.0 1.0

Notes:

*	The first line indicates that the following objects will be put in a group called "cube". Ex files must begin with a group declaration.

*	The second line says that zero dimensional nodes are to be read until a different shape is specified. You will seldom see this line in any .exnode file because 0-D nodes are the default shape at the start of file read and when any new group is started.

*	The next five lines up to the first Node is a node field header which declares finite element fields and indicates what field parameters will be read in with any nodes defined after the header. The first line of the header indicates only one field follows.

*	The first line following the #Fields declares a 3-component coordinate-type field called "coordinates" whose values are to be interpreted in a rectangular Cartesian coordinate system. This field defaults to having real values.

*	Following the declaration of the field are the details of the components including their names and the parameter values held for each, with the minimum being the value of the field at that node. The above node field component definitions indicate that there are no derivative parameters. The "Value index" is redundant since the index of where values for components "x", "y" and "z" of the "coordinates" field are held in each node"s parameter vector is calculated assuming they are in order (the correct index is written for interest).

*	Finally each of the nodes are listed followed by the number of parameters required from the node field header.

Example: multiple node fields and derivative parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A slightly more complex example adds a second field "temperature"::

  Region: /heated_bar
  #Fields=2
  1) coordinates, coordinate, rectangular cartesian, #Components=2
   x. Value index=1, #Derivatives=0
   y. Value index=2, #Derivatives=0
  2) temperature, field, rectangular cartesian, #Components=1
   1. Value index=3, #Derivatives=1 (d/ds1)
  Node: 1
   0.0 0.0
   37.0 0.0
  Node: 2
   1.0 0.0
   55.0 0.0
  Node: 3
   2.0 0.0
   80.2 0.0

Notes:

*	The coordinates field is now 2-dimensional. Beware it isn"t possible to have a two-component and three-component field of the same name in the same region.

*	The temperature field is of CM type field which merely means it has no special meaning (as opposed to the coordinate CM field type hint for the "coordinates"). Although it is not entirely relevant, the coordinate system must still be specified since cmgui performs appropriate transformations whenever it is used as a coordinate field.

*	The scalar (single component) temperature field has two parameters for each node, the first being the exact temperature at the node, the second being a nodal derivative. The optional text "(d/ds1)" labels the derivative parameter as the derivative of the temperature with respect to a physical distance in space. It isn"t until the definition of element interpolation that its contribution to the element field is known.

Example: node derivatives and versions in the prolate heart
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following snippet from example a/a3 shows the nodal parameters held at the apex of the prolate heart model::

  #Fields=2
   1) coordinates, coordinate, prolate spheroidal, focus=0.3525E+02, #Components=3
    lambda. Value index=1,#Derivatives=3 (d/ds1,d/ds2,d2/ds1ds2)
    mu. Value index=5, #Derivatives=0  
    theta. Value index=6,#Derivatives=0, #Versions=10
   2) fibres, anatomical, fibre, #Components=3
    fibre angle. Value index=16, #Derivatives=1 (d/ds1)
    imbrication angle. Value index=18, #Derivatives= 0  
    sheet angle. Value index=19, #Derivatives=3 (d/ds1,d/ds2,d2/ds1ds2)
  Node: 13
    0.984480E+00   0.000000E+00   0.000000E+00   0.000000E+00
    0.000000E+00
    0.253073E+00   0.593412E+00   0.933751E+00   0.127409E+01   0.188932E+01   0.250455E+01   0.373500E+01   0.496546E+01   0.558069E+01   0.619592E+01
   -0.138131E+01  -0.117909E+01
    0.000000E+00
   -0.827443E+00  -0.108884E+00  -0.245620E+00  -0.153172E-01

Notes:

*	This example uses a prolate spheroidal coordinate system for the coordinate field. This is inherently heart-like in shape allowing fewer parameters to describe the heart, and requires a focus parameter to set is scale and form.

*	The "theta" component of the prolate coordinates has 10 versions meaning there are 10 versions of each value and derivative specified. In this case there are no derivatives so only 10 values are read in. 10 versions are used to supply the angles at which each line element heads away from the apex of the heart which is on the axis of the prolate spheroidal coordinate system.

*	Node field parameters are always listed in component order, and for each component in the order:

|  value then derivatives for version 1
|  value then derivatives for version 2
|  etc.

*	The second field "fibres" declares a field of CM type anatomical with a fibre coordinate system. In elements these fields are interpreted as Euler angles for rotating an ortho-normal coordinate frame initially oriented with element "xi" axes and used to define axes of material anisotropy, such as muscle fibres in tissue.

Example: embedded locations in a mesh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Following is the file "cube_element_xi.exdata" taken from cmgui example a/ar (exnode formats), which defines a node field containing embedded locations within elements::

  Group name: xi_points
  #Fields=1
   1) element_xi, field, element_xi, #Components=1
    1. Value index=1, #Derivatives=0
  Node:     1
    E 1 3 0.25 0.25 0.75
  Node:     2
    E 1 3 0.25 0.5 0.75
  Node:     3
    E 1 3 1 0.25 0.75
  Node:     4
    E 1 3 1 1 1
  Node:     5
    E 1 3 0 0 0
  Node:     6
    F 3 2 0.3 0.6
  Node:     7
    L 1 1 0.4

Notes:

*	The field named "element_xi" uses the special value type confusingly also called element_xi indicating it returns a reference to an element and a location within its coordinate "xi" space. Only 1 component and no derivatives or versions are permitted with this value type.

*	Element xi values are written as:

|  *[region path] {element/face/line} number dimension xi-coordinates*
|  Where the region path is optional and element/face/line can be abbreviated to as little as one character, case insensitive. Hence node 1 lists  the location in three-dimensional element number 1 where xi=(0.25,0.25,0.75); node 6 lists a location in two-dimensional face element number 3 with face xi=(0.3,0.6).

*	Node fields storing embedded locations permit fields in the host element at those locations to be evaluated for the nodes using the special embedded computed field type, for example: "gfx define field embedded_coordinates embedded element_xi element_xi field element_coordinates".

Special field types
~~~~~~~~~~~~~~~~~~~

The example a/ar (exnode formats) and a/aq (exelem formats) lists several other special field types including constant (one value for all the nodes it is defined on) and indexed (value indexed by the value of a second integer "index field"), but their use is discouraged and they may be deprecated in time. If such functionality is required, prefer computed fields created with "gfx define field" commands or request indexed functionality on the cmgui tracker.

Defining elements and element fields
------------------------------------

Cmgui elements, shapes, faces and identifiers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Elements are objects comprising an n-dimensional (with n>0) coordinate space serving as a material coordinate system charting part or all of a body of interest. The set of elements covering the whole model is referred to as a mesh. We often use the Greek character "xi" to denote the coordinate within each element.

Each element has a shape which describes the form and limits of this coordinate space. Shapes are declared in the EX format are follows:

Shape. Dimension=n SHAPE-DESCRIPTION

Up to three dimensions, the most important shape descriptions are::

  Shape. Dimension=0				nodes (no shape)
  Shape. Dimension=1 line			line shape, xi covering [0,1]
  Shape. Dimension=2 line*line		square on [0,1]
  Shape. Dimension=2 simplex(2)*simplex	triangle on [0,1]; xi1+xi2<1
  Shape. Dimension=3 line*line*line		cube on [0,1]
  Shape. Dimension=3 simplex(2;3)*simplex*simplex
             tetrahedron on [0,1]; xi1+xi2+xi3<1
  Shape. Dimension=3 line*simplex(3)*simplex (and other permutations)
             triangle wedge; line on xi1 (etc.)

Pairs of dimensions can also be linked into polygon shapes; see example a/element_types.

The shape description works by describing the span of the space along each xi direction for its dimension. The simplest cases are the line shapes: "line*line*line" indicates the outer or tensor product of three line shapes, thus describing a cube. 
If the shape description is omitted then line shape is assumed for all dimensions. Simplex shapes, used for triangles and tetrahedra, cannot be simply described by an outer product and must be tied to another dimension; in the EX format the tied dimension is written in brackets after the first simplex coordinate, and for 3 or higher dimensional simplices all linked dimensions must be listed as shown for the tetrahedron shape.

The cmgui shape description is extensible to 4 dimensions or higher, for example the following denotes a 4-dimensional shape with a simplex across dimensions 1 and 2, and an unrelated simplex on dimensions 3 and 4::

  Shape. Dimension=4 simplex(2)*simplex*simplex(4)*simplex

Beware that while cmgui can in principle read such high-dimensional elements from exelem files, this has not been tested and few graphics primitives and other features of cmgui will be able to work with such elements.

Elements of a given shape have a set number of faces of dimension one less than their own. A cube element has 6 square faces at xi1=0, xi1=1, xi2=0, xi2=1, xi3=0, xi3=1. Each square element itself has 4 faces, each of line shape. The faces of elements are themselves elements, but they are identified differently from the real "top-level" elements.

The cmgui EX format uses a peculiar naming scheme for elements, consisting of 3 integers: the element number, the face number and the line number, only one of which should ever be non-zero. All elements over which fields are defined or which are not themselves the faces of a higher dimensional element use the first identifier, the element number. All 2-D faces of 3-D elements use the face number and all 1-D faces of 2-D elements (including faces of faces of 3-D elements) use the line number. The naming scheme for faces of 4 or higher dimensional elements is not yet defined. The element identifier "0 0 0" may be used to indicate a NULL face.

Element nodes, scale factors and parameter mappings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Cmgui elements store an array of nodes from which field parameters are extracted for interpolation by basis functions. This local node array can be as long as needed. It may contain repeated references to the same nodes, however it is usually preferable not to do this.

Cmgui elements can also have an array of real-valued scale factors for each basis function (see below).

These two arrays are combined in mapping global node parameters to an array of element field parameters ready to be multiplied by the basis function values to give the value of a field at any "xi" location in the element.

Mappings generally work by taking the field component parameter at index i from the node at local index j and multiplying it by the scale factor at index k for the basis function in-use for that field component. It is also possible to use a unit scale factor by referring to the scale factor at index 0, and to not supply a scale factor set if only unit scale factors are in use.

Global-to-local parameter mappings are at the heart of the complexity in cmgui element field definitions and are described in detail with the examples.

Cmgui element basis functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Basis functions are defined in cmgui ex format in a very similar manner to element shapes, by outer (tensor) product of the following functions along each xi axis::

  constant			constant
  l.Lagrange			linear Lagrange
  q.Lagrange			quadratic Lagrange
  c.Lagrange			cubic Lagrange
  c.Hermite			cubic Hermite
  LagrangeHermite		Lagrange at xi=0, Hermite at xi=1
  HermiteLagrange		Hermite at xi=0, Lagrange at xi=1
  l.simplex			linear simplex (see below)
  q.simplex			quadratic simplex (see below)
  polygon			piecewise linear around a polygon shape

It is not too difficult to extend these functions to higher order and to add other families of interpolation or other basis functions including the serendipity family, Bezier, Fourier etc. We are open to requests for new basis functions on the cmgui tracker.

Lagrange, Hermite, Bezier and many other 1-D basis functions are able to be combined in multiple dimensions by the outer product. This is not the case for simplex, polygon and serendipity families of basis functions which, like element shapes, require linked xi dimensions to be specified.

Some example element bases::

  l.Lagrange*l.Lagrange*l.Lagrange		trilinear interpolation (8 nodes)
  c.Hermite*c.Hermite				bicubic Hermite (4 nodes x 4 params)
  l.simplex(2)*l.simplex			linear triangle (3 nodes)
  q.simplex(2;3)*q.simplex*q.simplex	quadratic tetrahedron (10 nodes)
  c.Hermite*l.simplex(3)*l.simplex		cubic Hermite * linear triangle (6 nodes, 2 parameters per node)
  constant*constant*l.Lagrange		constant in xi1 and xi2, linearly varying in xi3

Most element bases have one basis functions per node which multiplies a single parameter obtained from that node. For instance, a linear Lagrange basis expects 2 nodes each with 1 parameter per field component. A bilinear Lagrange basis interpolates a single parameter from 4 nodes at the corners of a unit square. A 3-D linear-quadratic-cubic Lagrange element basis expects 2*3*4 nodes along the respective xi directions, with 1 basis function and one parameter for each node. A linear triangle has 3 nodes with 1 parameter each; a quadratic triangle has 6 nodes with 1 parameter.

1-D Hermite bases provide 2 basis functions per node, expected to multiple two parameters: (1) the value of the field at the node, and (2) the derivative of that field value with respect to the xi coordinate. If this derivative is common across an element boundary then the field is C¬1 continuous there. Outer products of 1-D Hermite basis functions double the number of parameters per node for each Hermite term. A bicubic Hermite basis expect 4 nodes with 4 basis functions per node, multiplying 4 nodal parameters: (1) value of the field, (2) the derivative of the field with respect to the first xi direction, (3) the derivative with respect to the second xi direction and (4) the double derivative of the field with respect to both directions referred to as the cross derivative. Tri-cubic Hermite bases have 8 basis functions per node, one multiplying the value, 3 for first derivatives, 3 for second (cross) derivatives and a final function multiplying a triple cross derivative parameter.

The ex format requires nodes contributing parameters for multiplication by a basis to be in a very particular order: changing fastest in xi1, then xi2, then xi3, and so on. Note this is not necessarily the order nodes are stored in the element node array, just the order in which those nodes are referenced in the parameter map. In most example files the order of the nodes in the element node list will also follow this pattern.

Cmgui example a/element_types provides a large number of sample elements using complex combinations of basis functions on supported 3-D element shapes.

Example: tri-linear interpolation on a cube
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Following is an example of a coordinate field defined over a unit cube, adapted from example a/a2, with faces and scale factors removed to cut the example down to minimum size, and assuming the cube node file from earlier has already been loaded::

  Region: /cube
  Shape.  Dimension=3  line*line*line
  #Scale factor sets=0
  #Nodes=8
  #Fields=1
   1) coordinates, coordinate, rectangular cartesian, #Components=3
     x.  l.Lagrange*l.Lagrange*l.Lagrange, no modify, standard node based.
     #Nodes= 8
      1.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      2.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      3.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      4.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      5.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      6.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      7.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      8.  #Values=1
       Value indices:     1
       Scale factor indices:   0
     y.  l.Lagrange*l.Lagrange*l.Lagrange, no modify, standard node based.
     #Nodes= 8
      1.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      2.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      3.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      4.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      5.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      6.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      7.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      8.  #Values=1
       Value indices:     1
       Scale factor indices:   0
     z.  l.Lagrange*l.Lagrange*l.Lagrange, no modify, standard node based.
     #Nodes= 8
      1.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      2.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      3.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      4.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      5.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      6.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      7.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      8.  #Values=1
       Value indices:     1
       Scale factor indices:   0
   Element:     1 0 0
     Nodes:
       1     2     3     4     5     6     7     8

Notes:

*	"Shape. Dimension=3 line*line*line" declares that following this point three dimensional cube-shaped elements are being defined. "line*line*line" could have been omitted as line shapes are the default.

*	"#Scale factor sets=0" indicates no scale factors are to be read in with the elements that follow. Scale factors are usually only needed for Hermite basis functions when nodal derivative parameters are maintained with respect to a physical distance and the scale factors convert the derivative to be with respect to the element xi coordinate. Avoid scale factors when not needed.

*	The 4th line "#Nodes=8" says that 8 nodes will be listed with all elements defined under this header.

*	Under the declaration of the "coordinates" field (which is identical to its declaration for the nodes) are the details on how the field is evaluated for each field component. Each field component is always described separately: each may use different basis functions and parameter mappings. In this example the components "x", "y" and "z" all use the same tri-linear basis functions and use an identical parameter mapping except that parameters are automatically taken from the corresponding field component at the node.

*	"no modify" in the element field component definition is an instruction to do no extra value manipulations as part of the interpolation. Other modify instructions resolve the ambiguity about which direction one interpolates angles in polar coordinates. The possible options are "increasing in xi1", "decreasing in xi1", "non-increasing in xi1", "non-decreasing in xi1" or "closest in xi1". For use, see the prolate heart example later.

*	"standard node based" indicates that each element field parameter is obtained by multiplying one parameter extracted from a node by one scale factor. An alternative called "general node based" evaluates each element field parameter as the dot product of multiple node field parameters and scale factors; it is currently unimplemented for I/O but is designed to handle the problems like meshing the apex of the heart with Hermite basis functions without requiring multiple versions. A final option "grid based" is described in a later example.

*	Following the above text are several lines describing in detail how all the element field parameters are evaluated prior to multiplication by the basis functions. Being a tri-linear Lagrange basis, 8 nodes must be accounted for:

``#Nodes= 8``

Following are 8 sets of three lines each indicating the index of the node in the element"s node array from which parameters are extracted (in this case the uncomplicated sequence 1,2,3,4,5,6,7,8), the number of values to be extracted (1), the index of the each parameter value in the list of parameters for that field component at that node and the index of the scale factor multiplying it from the scale factor array for that basis, or zero to indicate a unit scale factor::

  1.  #Values=1
   Value indices:     1
   Scale factor indices:   0
   
As mentioned earlier, the nodes listed in the mapping section must always follow a set order, increasing in xi1 fastest, then xi2, then xi3, etc. to match the order of the basis functions procedurally generated from the basis description.

*	At the end of the example is the definition of element "0 0 1" which lists the nodes 1 to 8.

Defining faces and lines
~~~~~~~~~~~~~~~~~~~~~~~~

The above example is not ready to be fully visualised in cmgui because it contains no definitions of element faces and lines. Cmgui requires these to be formally defined because it visualises element surfaces by evaluating fields on face elements, and element edges via line elements. The command "gfx define faces ..." can create the faces and lines after loading such a file. Alternatively they can be defined and referenced within the ex file as in the following::

  Region: /cube
   Shape.  Dimension=1
   Element: 0 0     1
   Element: 0 0     2
   Element: 0 0     3
   Element: 0 0     4
   Element: 0 0     5
   Element: 0 0     6
   Element: 0 0     7
   Element: 0 0     8
   Element: 0 0     9
   Element: 0 0    10
   Element: 0 0    11
   Element: 0 0    12
  Shape.  Dimension=2
   Element: 0     1 0
     Faces:
     0 0     3
     0 0     7
     0 0     2
     0 0    10
   Element: 0     2 0
     Faces:
     0 0     5
     0 0     8
     0 0     4
     0 0    11
   Element: 0     3 0
     Faces:
     0 0     1
     0 0     9
     0 0     3
     0 0     5
   Element: 0     4 0
     Faces:
     0 0     6
     0 0    12
     0 0     7
     0 0     8
   Element: 0     5 0
     Faces:
     0 0     2
     0 0     4
     0 0     1
     0 0     6
   Element: 0     6 0
     Faces:
     0 0    10
     0 0    11
     0 0     9
     0 0    12
  Shape.  Dimension=3
   Element:     1 0 0
     Faces: 
     0     1 0
     0     2 0
     0     3 0
     0     4 0
     0     5 0
     0     6 0
     Nodes:
       1     2     3     4     5     6     7     8

Likewise scale factors can be read in as listed in the cube.exelem file from cmgui example a/a2, however with Lagrange basis functions the scale factors are all unit valued, so this is rather needless.

Example: collapsed square element
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following tricky example collapses a square element to a triangle by using the third local node twice::

  Region: /collapse
  Shape. Dimension=0
  #Fields=1
  1) coordinates, coordinate, rectangular cartesian, #Components=2
   x. Value index=1, #Derivatives=0
   y. Value index=2, #Derivatives=0
  Node: 1
   0.0 0.0
  Node: 2
   1.0 0.0
  Node: 3
   0.5 1.0
  Shape.  Dimension=1  line
   Element: 0 0 1
   Element: 0 0 2
   Element: 0 0 3
  Shape.  Dimension=2  line*line
  #Scale factor sets=0
  #Nodes=3
  #Fields=1
   1) coordinates, coordinate, rectangular cartesian, #Components=2
     x.  l.Lagrange*l.Lagrange, no modify, standard node based.
     #Nodes= 4
      1.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      2.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      3.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      3.  #Values=1
       Value indices:     1
       Scale factor indices:   0
     y.  l.Lagrange*l.Lagrange, no modify, standard node based.
     #Nodes= 4
      1.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      2.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      3.  #Values=1
       Value indices:     1
       Scale factor indices:   0
      3.  #Values=1
       Value indices:     1
       Scale factor indices:   0
  Element:     1 0 0
     Faces: 
     0 0 1
     0 0 2
     0 0 3
     0 0 0
     Nodes:
       1     2     3

Notes:

*	Element 1 0 0 has a node array with only 3 nodes in it, but the third and fourth parameter mappings both refer to the node at index 3 in the element node list.

*	Note that face on the collapsed side of the element is undefined, as indicated by special face identifier 0 0 0.

*	It is also possible to obtain an equivalent result by physically storing 4 nodes in the element but repeating node 3 in that array.

Simplex elements: triangles and tetrahedra
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The cmgui example a/testing/simplex defines two fields on a triangle element, one using a 6-node quadratic simplex, the other a 3-node linear simplex basis. The element in that example stores 6 nodes in its node array, only 3 of which are used for the linear basis, but all 6 contribute parameters to the quadratic interpolation.

Example a/element_types has both linear and quadratic tetrahedra.

Example: multiple fields, bases and scale factor sets in the prolate heart
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

At the more complex end of the scale is this excerpt from the prolate heart model from cmgui example a/a3. It defines two element fields using different basis functions for each field component. It was exported from cm which always uses a full complement of scale factors i.e. one per basis function.

::

  Shape.  Dimension=3
   #Scale factor sets= 4
     c.Hermite*c.Hermite*l.Lagrange, #Scale factors=32
     l.Lagrange*l.Lagrange*l.Lagrange, #Scale factors=8
     l.Lagrange*l.Lagrange*c.Hermite, #Scale factors=16
     l.Lagrange*c.Hermite*c.Hermite, #Scale factors=32
   #Nodes=           8
   #Fields=2
   1) coordinates, coordinate, prolate spheroidal, focus=  0.3525E+02, #Components=3
     lambda.  c.Hermite*c.Hermite*l.Lagrange, no modify, standard node based.
     #Nodes= 8
      1.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:   1   2   3   4
      2.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:   5   6   7   8
      3.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:   9  10  11  12
      4.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  13  14  15  16
      5.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  17  18  19  20
      6.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  21  22  23  24
      7.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  25  26  27  28
      8.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  29  30  31  32
     mu.  l.Lagrange*l.Lagrange*l.Lagrange, no modify, standard node based.
     #Nodes= 8
      1.  #Values=1
       Value indices:     1
       Scale factor indices:  33
      2.  #Values=1
       Value indices:     1
       Scale factor indices:  34
      3.  #Values=1
       Value indices:     1
       Scale factor indices:  35
      4.  #Values=1
       Value indices:     1
       Scale factor indices:  36
      5.  #Values=1
       Value indices:     1
       Scale factor indices:  37
      6.  #Values=1
       Value indices:     1
       Scale factor indices:  38
      7.  #Values=1
       Value indices:     1
       Scale factor indices:  39
      8.  #Values=1
       Value indices:     1
       Scale factor indices:  40
     theta.  l.Lagrange*l.Lagrange*l.Lagrange, decreasing in xi1, standard node based.
     #Nodes= 8
      1.  #Values=1
       Value indices:     1
       Scale factor indices:  33
      2.  #Values=1
       Value indices:     1
       Scale factor indices:  34
      3.  #Values=1
       Value indices:     1
       Scale factor indices:  35
      4.  #Values=1
       Value indices:     1
       Scale factor indices:  36
      5.  #Values=1
       Value indices:     1
       Scale factor indices:  37
      6.  #Values=1
       Value indices:     1
       Scale factor indices:  38
      7.  #Values=1
       Value indices:     1
       Scale factor indices:  39
      8.  #Values=1
       Value indices:     1
       Scale factor indices:  40
   2) fibres, anatomical, fibre, #Components=3
     fibre angle.  l.Lagrange*l.Lagrange*c.Hermite, no modify, standard node based.
     #Nodes= 8
      1.  #Values=2
       Value indices:     1   2
       Scale factor indices:  41  42
      2.  #Values=2
       Value indices:     1   2
       Scale factor indices:  43  44
      3.  #Values=2
       Value indices:     1   2
       Scale factor indices:  45  46
      4.  #Values=2
       Value indices:     1   2
       Scale factor indices:  47  48
      5.  #Values=2
       Value indices:     1   2
       Scale factor indices:  49  50
      6.  #Values=2
       Value indices:     1   2
       Scale factor indices:  51  52
      7.  #Values=2
       Value indices:     1   2
       Scale factor indices:  53  54
      8.  #Values=2
       Value indices:     1   2
       Scale factor indices:  55  56
     imbrication angle.  l.Lagrange*l.Lagrange*l.Lagrange, no modify, standard node based.
     #Nodes= 8
      1.  #Values=1
       Value indices:     1
       Scale factor indices:  33
      2.  #Values=1
       Value indices:     1
       Scale factor indices:  34
      3.  #Values=1
       Value indices:     1
       Scale factor indices:  35
      4.  #Values=1
       Value indices:     1
       Scale factor indices:  36
      5.  #Values=1
       Value indices:     1
       Scale factor indices:  37
      6.  #Values=1
       Value indices:     1
       Scale factor indices:  38
      7.  #Values=1
       Value indices:     1
       Scale factor indices:  39
      8.  #Values=1
       Value indices:     1
       Scale factor indices:  40
     sheet angle.  l.Lagrange*c.Hermite*c.Hermite, no modify, standard node based.
     #Nodes= 8
      1.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  57  58  59  60
      2.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  61  62  63  64
      3.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  65  66  67  68
      4.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  69  70  71  72
      5.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  73  74  75  76
      6.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  77  78  79  80
      7.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  81  82  83  84
      8.  #Values=4
       Value indices:     1   2   3   4
       Scale factor indices:  85  86  87  88
   Element:            1 0 0
     Faces: 
     0     1 0
     0     2 0
     0     3 0
     0     4 0
     0     5 0
     0     6 0
     Nodes:
           19           82           14           83            5           52            1           53
     Scale factors:
       0.1000000000000000E+01   0.2531332864778986E+02   0.3202170161207646E+02   0.8105758567679540E+03   0.1000000000000000E+01
     0.2540127674788437E+02   0.3851739595427941E+02   0.9783910342424932E+03   0.1000000000000000E+01   0.2665607536220107E+02
     0.2913687357203342E+02   0.7766746977550476E+03   0.1000000000000000E+01   0.2797776438705370E+02   0.3675988075068424E+02
     0.1028459282538834E+04   0.1000000000000000E+01   0.3107367883817446E+02   0.3665266951220884E+02   0.1138933280984126E+04
     0.1000000000000000E+01   0.3053066581298630E+02   0.4220277992007600E+02   0.1288478970118849E+04   0.1000000000000000E+01
     0.3612724280632425E+02   0.3339669014010959E+02   0.1206530333619314E+04   0.1000000000000000E+01   0.3620256762563091E+02
     0.3810870609422361E+02   0.1379633009501423E+04
       0.1000000000000000E+01   0.1000000000000000E+01   0.1000000000000000E+01   0.1000000000000000E+01   0.1000000000000000E+01
     0.1000000000000000E+01   0.1000000000000000E+01   0.1000000000000000E+01
       0.1000000000000000E+01   0.8802929891392392E+01   0.1000000000000000E+01   0.7673250860396258E+01   0.1000000000000000E+01
     0.1368084332227282E+02   0.1000000000000000E+01   0.1181772996260416E+02   0.1000000000000000E+01   0.8802929891392392E+01
     0.1000000000000000E+01   0.7673250860396258E+01   0.1000000000000000E+01   0.1368084332227282E+02   0.1000000000000000E+01
     0.1181772996260416E+02
       0.1000000000000000E+01   0.3202170161207646E+02   0.8802929891392392E+01   0.2818847942941958E+03   0.1000000000000000E+01
     0.3851739595427941E+02   0.7673250860396258E+01   0.2955536416463978E+03   0.1000000000000000E+01   0.2913687357203342E+02
     0.1368084332227282E+02   0.3986170022398609E+03   0.1000000000000000E+01   0.3675988075068424E+02   0.1181772996260416E+02
     0.4344183441691171E+03   0.1000000000000000E+01   0.3665266951220884E+02   0.8802929891392392E+01   0.3226508800483498E+03
     0.1000000000000000E+01   0.4220277992007600E+02   0.7673250860396258E+01   0.3238325173328371E+03   0.1000000000000000E+01
     0.3339669014010959E+02   0.1368084332227282E+02   0.4568948852893329E+03   0.1000000000000000E+01   0.3810870609422361E+02
     0.1181772996260416E+02   0.4503583978457821E+03

Notes:

*	It can be seen that for each Hermite term in the basis function there are twice as many parameter values for each node.

*	The "Value indices" are indices into the array of parameters held for the field component at the node, starting at 1 for the value. It is not possible to refer to parameters by names such as "value", "d/ds1" or by version number.

* The "decreasing in xi1" option specified for the theta component of the coordinates field specifies that as xi increases, the angle of theta decreases. This needs to be stated since it is equally possible to interpolate this angle in the opposite direction around the circle.

*	All scale factors are listed in a single block; in this case there are 32+8+16+32=88 scale factors, listed in the order of the scale factor set declaration at the top of the file excerpt. Scale factor indices are absolute locations in this array, but they are considered invalid if referring to parts of the array not containing scale factors for the basis used in the field component.

*	Note how all the scale factors for the tri-linear Lagrange basis are equal to 1.0.

Example: per-element constant and grid-based element fields
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Cmgui and its EX files also support storage of regular grids of real or integer values across elements. The grid is assumed regular across N divisions on lines, N*M divisions on squares and N*M*P divisions on cubes.

Per-element constants are a special case using constant bases together with 0 grid divisions. These have only been supported since Cmgui 2.7 (April 2010). See this extract from cmgui example a/element_constants::

  1) temperature, field, rectangular cartesian, #Components=1
   value. constant*constant*constant, no modify, grid based.
   #xi1=0, #xi2=0, #xi3=0
  Element: 1 0 0
    Values :
    48.0

Linear Lagrange interpolation is used when there are 1 or more element divisions. Following is an excerpt from cmgui example a/aq "element formats"::

  Group name: block
  Shape.  Dimension=3
  #Scale factor sets=0
  #Nodes=0
  #Fields=2
  1) material_type, field, integer, #Components=1
   number. l.Lagrange*l.Lagrange*l.Lagrange, no modify, grid based.
   #xi1=2, #xi2=3, #xi3=2
  2) potential, field, real, #Components=1
   value. l.Lagrange*l.Lagrange*l.Lagrange, no modify, grid based.
   #xi1=2, #xi2=3, #xi3=2
  Element: 1 0 0
    Values:
    1 1 3
    1 1 3
    1 2 3
    1 2 2
    1 1 3
    1 1 3
    1 2 3
    2 2 2
    1 3 3
    1 3 3
    2 2 3
    2 2 2
    13.5 12.2 10.1
    14.5 12.2 10.1
    15.5 12.2 10.1
    16.5 12.2 10.1
    12.0 11.0 10.0
    13.0 11.0 10.0
    14.0 11.0 10.0
    15.0 11.0 10.0
    10.5 10.7 9.9
    11.5 10.7 9.9
    12.5 10.7 9.9
    13.5 10.7 9.9

Notes:

*	"#xi1=2, #xi2=3, #xi3=2" actually refers to the number of divisions between grid points, so 3*4*3=36 values are read in, and represent values at the corners of the grid "cells". If there are 2 divisions along an xi direction, values are held for xi=0.0, xi=0.5 and xi=1.0. Under each element, values are listed in order of location changing fastest along xi1, then along xi2, then along xi3.

*	Only constant (for number-in-xi = 0) or linear Lagrange bases are supported. The basis is irrelevant for integer-valued grids which choose the "nearest value", so halfway between integer value 1 and 3 the field value jumps directly from 1 to 3.

*	Grid point values along boundaries of adjacent elements must be repeated in each element.

Further information
-------------------

Further information about cmgui and its data formats can be found on the following web-site:

http://www.cmiss.org/cmgui

We also invite questions, bug reports and feature requests under the tracker at:

https://tracker.physiomeproject.org

Note: be sure to search and post under the "cmgui" project!

