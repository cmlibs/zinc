"""
PyZinc Unit Tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

'''
Created on May 23, 2013

@author: Alan Wu
'''
import unittest

from opencmiss.zinc.context import Context
from opencmiss.zinc.sceneviewer import Sceneviewer
import opencmiss.zinc.scenecoordinatesystem

class SceneviewerProjectionFieldTestCase(unittest.TestCase):


    def setUp(self):
        self.context = Context("sceneviewerprojectionstest")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldmodule()


    def tearDown(self):
        del self.field_module
        del self.context


    def testSceneviewerProjectionFieldCreate(self):
        self.assertRaises(TypeError, self.field_module.createFieldSceneviewerProjection, [1])
        scene_viewer_module = self.context.getSceneviewermodule()
        scene_viewer = scene_viewer_module.createSceneviewer(Sceneviewer.BUFFERING_MODE_DOUBLE, Sceneviewer.STEREO_MODE_DEFAULT)
        f1 = self.field_module.createFieldSceneviewerProjection(scene_viewer,
            opencmiss.zinc.scenecoordinatesystem.SCENECOORDINATESYSTEM_LOCAL,
            opencmiss.zinc.scenecoordinatesystem.SCENECOORDINATESYSTEM_WINDOW_PIXEL_TOP_LEFT)
        self.assertTrue(f1.isValid())

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(SceneviewerProjectionFieldTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
