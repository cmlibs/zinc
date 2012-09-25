
from PyQt4 import QtCore, QtGui, QtOpenGL
from Zn import Context, Region, GraphicsModule, Graphic
from Zn import SceneViewer, Rendition, Scene, GraphicsFilter

class QtGraphicsCanvas(QtOpenGL.QGLWidget):
    def __init__(self, parent = None):
        QtOpenGL.QGLWidget.__init__(self, parent)
        self.context_ = Context.Context("axisviewer")
        self.scene_viewer_ = None

    def initializeGL(self):
        scene_viewer_package_ = self.context_.getDefaultSceneViewerPackage()
        self.scene_viewer_ = scene_viewer_package_.createSceneViewer(SceneViewer.SceneViewer.BUFFERING_MODE_DOUBLE, SceneViewer.SceneViewer.STEREO_MODE_ANY)
        root_region = self.context_.getDefaultRegion()
        graphics_module = self.context_.getDefaultGraphicsModule()
        graphics_module.enableRenditions(root_region)
        rendition = graphics_module.getRendition(root_region)
        rendition.beginChange()
        graphics_filter = graphics_module.createFilterVisibilityFlags()
        scene = graphics_module.createScene()
        scene.setRegion(root_region)
        scene.setFilter(graphics_filter)
        graphic = rendition.createGraphic(Rendition.Rendition.GRAPHIC_POINT)
        graphic.setGlyphType(Graphic.Graphic.GLYPH_TYPE_AXES)
        self.scene_viewer_.setScene(scene)
        rendition.endChange()
        
       
    def paintGL(self):
        self.scene_viewer_.redrawNow()

    def resizeGL(self, width, height):
        self.scene_viewer_.setViewportSize(width, height)

    def mousePressEvent(self, mouseevent):
        input = self.scene_viewer_.getInput()
        input.setPosition(mouseevent.x(), mouseevent.y())
        input.setType(SceneViewer.SceneViewerInput.INPUT_EVENT_TYPE_BUTTON_PRESS)
        if mouseevent.button() == QtCore.Qt.LeftButton:
            input.setButtonNumber(1)
        elif mouseevent.button() == QtCore.Qt.MiddleButton:
            input.setButtonNumber(2)
        elif mouseevent.button() == QtCore.Qt.RightButton:
            input.setButtonNumber(3)
            
        self.scene_viewer_.defaultInputCallback(input)
        
    def mouseReleaseEvent(self, mouseevent):
        input = self.scene_viewer_.getInput()
        input.setPosition(mouseevent.x(), mouseevent.y())
        input.setType(SceneViewer.SceneViewerInput.INPUT_EVENT_TYPE_BUTTON_RELEASE)
        if mouseevent.button() == QtCore.Qt.LeftButton:
            input.setButtonNumber(1)
        elif mouseevent.button() == QtCore.Qt.MiddleButton:
            input.setButtonNumber(2)
        elif mouseevent.button() == QtCore.Qt.RightButton:
            input.setButtonNumber(3)
            
        self.scene_viewer_.defaultInputCallback(input)
        
    def mouseMoveEvent(self, mouseevent):
        input = self.scene_viewer_.getInput()
        input.setPosition(mouseevent.x(), mouseevent.y())
        input.setType(SceneViewer.SceneViewerInput.INPUT_EVENT_TYPE_MOTION_NOTIFY)
        if mouseevent.type() == QtCore.QEvent.Leave:
            input.setPosition(-1, -1)
        
        self.scene_viewer_.defaultInputCallback(input)
        
        self.updateGL()


