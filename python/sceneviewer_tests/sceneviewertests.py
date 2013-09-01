'''
Created on Aug 31, 2013

@author: hsorby
'''
import unittest, math

from zinc.context import Context
from zinc.sceneviewer import SceneViewer

class SceneViewerTestCase(unittest.TestCase):

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
        scene_viewer_module = self.graphics_module.getSceneViewerModule()
        self.scene_viewer = scene_viewer_module.createSceneViewer(SceneViewer.BUFFERING_MODE_DOUBLE, SceneViewer.STEREO_MODE_ANY)
        params = self.scene_viewer.getLookatParameters()
        self.assertEqual([0, 0, 2], params[1])
        self.assertEqual([0, 0, 0], params[2])
        self.assertEqual([0, 1, 0], params[3])
        
        self.scene_viewer.setLookatParametersNonSkew([-3, 7, 5], [9, -11, 13], [13, 1, 2.5])
        params = self.scene_viewer.getLookatParameters()
        self.assertEqual([-3, 7, 5], params[1])
        self.assertEqual([9, -11, 13], params[2])
        self.assertAlmostEqual(0.8297562372933929, params[3][0], 8)
        self.assertAlmostEqual(0.5580193340841622, params[3][1], 8)
        self.assertAlmostEqual(0.0109091457492757, params[3][2], 8)

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(SceneViewerTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
