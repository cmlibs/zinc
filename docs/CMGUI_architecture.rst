CMGUI Architecture
==================

Regions and Fields
------------------

The basic structural unit in CMGUI is called a *region* - region is a container for the objects representing a model.  Regions own a set of fields and may contain child regions, thereby allowing a hierarchical model structure.  For example, you may have a "body" region that contains "heart", "lung", and "stomach" regions.  

When you start CMGUI, a *root region* exists; this is an empty space into which other regions are loaded.  When you load in exnode and exelem files, a new region will be created in the *root region* named for the group declared in the files.  For example, if you were to load in heart.exnode and heart.exelem files which declare a group "heart" (via the line ``Group name: heart``), a new region called "heart" would be created containing all the nodes, elements and fields defined in these files.

Each region contains a set of *fields* from which a model is built up.  A field is a function that returns values over a domain; for example, the "potential" field could provide a value for the electrical potential of each point in the heart domain, which could be a finite element mesh.  The geometry of the heart is itself defined by the coordinate field (this field is often called "coordinates" in our models).  Finite element fields in CMGUI are generally defined by storing discrete parameters at nodes (eg the coordinates of the node, the electrical potential at each node) and interpolated across the *xi space* spanned by elements to give the continuous field.

Other examples of fields include fibre orientation in the heart, temperature, pressure, material properties, strain and so on.  In CMGUI you can also define fields that are computed from other fields.  For example, you might define a field called "scaled_coordinates" by scaling coordinates x, y, and z to x, y, and 10z, perhaps to exaggerate the height of topological features.  You might also compute strain from deformed and undeformed coordinates, fibre axis from fibre Euler angles, and so on.

Other Data in CMGUI
-------------------

The structure of the CMGUI *command data* (that is, the complete set of data used by CMGUI) is divided into a number of *containers* and categories.  Besides the regions which contain fields, there are also the following lists and managers, which contain other data used to create visualizations:

* Scene manager - scenes contain definitions of how to display visualizations.  The region created when you load exnode and exelem data (such as the heart region described above) is used to automatically create a graphical representation in scene "default" when loaded.  Scenes contains either (1) graphical renditions of regions (2) graphics objects (3) child scenes.

* Graphical material manager - contains a list of graphical materials that can be applied to graphical representations or objects.

* Texture manager - contains a list of textures that can be used in material definitions or volume renderings, etc.  Textures are 2D or 3D images.

* Spectrum manager - contains a list of spectra that can be used in material definitions.  These control how the graphical material is changed when graphics are coloured by a data field. 

* Graphics object list - contains simple graphics objects which are not tied to a model representation, but can be drawn in a scene.  These can animate with time.

* Glyph list - contains glyphs that can be used to represent data points in the graphics window - such things as spheres, cubes, arrows, axes, points or lines.  Objects from the graphics objects list may be added to the glyph list, in order to create customized glyphs.

CMGUI is not like most common software packages that can save a single file containing all of your work. In CMGUI, the data being worked on is often loaded in as a number of separate files, and the editing of the visual representation of this data often does not change it; it only alters how it is represented in the 3D window.  Currently, it is not possible to save your work in a single file that can be loaded in order to recreate all your work on the representation. For example; if you load in a model, change the viewpoint, alter the materials used to render it, and add glyphs to important data points, there is no way to simply "save" all of these changes from a simple menu item. There are ways to save all these changes but this will be covered in another section.