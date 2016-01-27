"""
PyZinc Unit Tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

'''
Created on Aug 31, 2013

@author: hsorby
'''
import unittest

from opencmiss.zinc.context import Context
from opencmiss.zinc.sceneviewer import Sceneviewer
from opencmiss.zinc.sceneviewerinput import Sceneviewerinput
from opencmiss.zinc import status

class SceneviewerTestCase(unittest.TestCase):

    def setUp(self):
        self.context = Context('sceneviewertest')
        root_region = self.context.getDefaultRegion()
        self.scene = root_region.getScene()


    def tearDown(self):
        del self.scene
        del self.context


    def testLookAtParameters(self):
        scene_viewer_module = self.context.getSceneviewermodule()
        scene_viewer = scene_viewer_module.createSceneviewer(Sceneviewer.BUFFERING_MODE_DOUBLE, Sceneviewer.STEREO_MODE_DEFAULT)
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
        scene_viewer_module = self.context.getSceneviewermodule()
        scene_viewer = scene_viewer_module.createSceneviewer(Sceneviewer.BUFFERING_MODE_DOUBLE, Sceneviewer.STEREO_MODE_DEFAULT)
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
        scene_viewer_module = self.context.getSceneviewermodule()
        scene_viewer = scene_viewer_module.createSceneviewer(Sceneviewer.BUFFERING_MODE_DOUBLE, Sceneviewer.STEREO_MODE_DEFAULT)
        scene_viewer_input = scene_viewer.createSceneviewerinput()
        self.assertEqual(status.OK, scene_viewer_input.setPosition(3, 5))
        self.assertEqual(status.OK, scene_viewer_input.setButtonType(Sceneviewerinput.BUTTON_TYPE_LEFT))
        self.assertEqual(status.OK, scene_viewer_input.setEventType(Sceneviewerinput.EVENT_TYPE_BUTTON_PRESS))
        self.assertEqual(status.OK, scene_viewer_input.setModifierFlags(Sceneviewerinput.MODIFIER_FLAG_CONTROL | Sceneviewerinput.MODIFIER_FLAG_SHIFT))

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(SceneviewerTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
