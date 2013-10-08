'''
Created on Aug 31, 2013

@author: hsorby
'''
import unittest

from opencmiss.zinc.context import Context
from opencmiss.zinc.sceneviewer import Sceneviewer, Sceneviewerinput
from opencmiss.zinc import status

class SceneviewerTestCase(unittest.TestCase):

    def setUp(self):
        self.context = Context('sceneviewertest')
        root_region = self.context.getDefaultRegion()
        self.graphics_module = self.context.getGraphicsModule()
        self.scene = self.graphics_module.getScene(root_region)


    def tearDown(self):
        del self.graphics_module
        del self.scene
        del self.context


    def testLookAtParameters(self):
        scene_viewer_module = self.graphics_module.getSceneviewermodule()
        scene_viewer = scene_viewer_module.createSceneviewer(Sceneviewer.BUFFERING_DOUBLE, Sceneviewer.STEREO_ANY_MODE)
        params = scene_viewer.getLookatParameters()
        self.assertEqual([0, 0, 2], params[1])
        self.assertEqual([0, 0, 0], params[2])
        self.assertEqual([0, 1, 0], params[3])
        
        scene_viewer.setLookatParametersNonSkew([-3, 7, 5], [9, -11, 13], [13, 1, 2.5])
        params = scene_viewer.getLookatParameters()
        self.assertEqual([-3, 7, 5], params[1])
        self.assertEqual([9, -11, 13], params[2])
        self.assertAlmostEqual(0.8297562372933929, params[3][0], 8)
        self.assertAlmostEqual(0.5580193340841622, params[3][1], 8)
        self.assertAlmostEqual(0.0109091457492757, params[3][2], 8)
        
    def testLookAtParametersIndividual(self):
        scene_viewer_module = self.graphics_module.getSceneviewermodule()
        scene_viewer = scene_viewer_module.createSceneviewer(Sceneviewer.BUFFERING_DOUBLE, Sceneviewer.STEREO_ANY_MODE)
        self.assertEqual([0, 0, 2], scene_viewer.getEyePosition()[1])
        self.assertEqual([0, 0, 0], scene_viewer.getLookatPosition()[1])
        self.assertEqual([0, 1, 0], scene_viewer.getUpVector()[1])
        
        self.assertEqual(status.OK, scene_viewer.setEyePosition([3, 2, 1]))
        self.assertEqual(status.OK, scene_viewer.setLookatPosition([3, 6, 1]))
        self.assertEqual(status.OK, scene_viewer.setUpVector([0, 0, 3]))
        self.assertEqual([3, 2, 1], scene_viewer.getEyePosition()[1])
        self.assertEqual([3.0, 6.0, 1.0], scene_viewer.getLookatPosition()[1])
        self.assertEqual([0, 0, 1], scene_viewer.getUpVector()[1])
        
    def testSceneviewerinput(self):
        scene_viewer_module = self.graphics_module.getSceneviewermodule()
        scene_viewer = scene_viewer_module.createSceneviewer(Sceneviewer.BUFFERING_DOUBLE, Sceneviewer.STEREO_ANY_MODE)
        scene_viewer_input = scene_viewer.createSceneviewerinput()
        self.assertEqual(status.OK, scene_viewer_input.setPosition(3, 5))
        self.assertEqual(status.OK, scene_viewer_input.setButton(Sceneviewerinput.BUTTON_LEFT))
        self.assertEqual(status.OK, scene_viewer_input.setEventType(Sceneviewerinput.EVENT_BUTTON_PRESS))
        self.assertEqual(status.OK, scene_viewer_input.setModifiers(Sceneviewerinput.MODIFIER_CONTROL | Sceneviewerinput.MODIFIER_SHIFT))

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(SceneviewerTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
