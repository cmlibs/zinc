CMGUI Material Editor Window
============================

The material editor window is where you define materials to be applied to graphical elements or objects in the graphics window.  Along the top of the material editor window are three buttons; create, delete and rename.  You can create a new material, delete or rename an existing material using these buttons.  Below these buttons is the list of currently defined materials.  These will contain the default materials, as well as any defined in any comfile that has been run.

Below the material list is a panel containing four colour editors.  These control the ambient, diffuse, emitted and specular colours.

* Ambient - The ambient colour is the "unlit" colour, or the colour of parts of the object that are in shadow.
* Diffuse - This is the overall colour of the material, the colour that the lit parts of the object will appear.
* Emitted - The emitted colour is the "glow" of a material; this colour will appear in both the lit and unlit parts of the material.
* Specular - This is the colour of the shine that appears on the material.  This shine appears as a glossy highlight.

Each colour can be edited using three sliders or textboxes, and you can choose from three colourspaces for editing the colour.  The default is RGB, but the HSV and CMY colourspaces are also available from a pull-down menu at the top of each colour editor.  Each colour editor has a preview panel showing a flat sample of the colour that is being edited.

Below the colour editors is the surface editor.  This panel allows you to set the alpha, shininess, and texture properties of the surface of the material being edited.  The alpha value sets the transparency of the material.  The shininess sets the "tightness" or size of the specular highlights of a material; generally the higher the shininess, the smaller and harder-edged the highlights.  Higher shininess makes a material look glossier.  The surface editor also allows you to assign a texture to the material surface - this option is unavailable unless you have created at least one texture by using the "gfx create texture" command to read in some bitmapped graphics.  There are two other check box options available in the surface editor: "per pixel shading" which greatly increases the quality of the lighting of the material, and bump mapping which allows you to use a texture file to create surface details. These options are only available on hardware that supports advanced forms of shading.

Below the surface editor is a panel that shows a preview of the currently edited material applied to a sphere.  You may need to resize the material editor window in order to see a usefully large preview panel.  Clicking in the preview panel will cycle the background from the RGB colours through black, white and back to RGB.

At the bottom of the window are four buttons: OK, apply, revert and cancel.  The OK button leaves the editor and applies the changes made.  The apply button immediately applies current changes to the materials, allowing you to see how they look if they are used in any objects shown in the graphics window.  The revert button will undo any changes made to the currently edited material, and the cancel button exits the material editor window without applying changes.


.. figure:: materialeditorwindow.png
   :figwidth: image
   :align: center

   **Figure 1: The CMGUI Material Editor**
