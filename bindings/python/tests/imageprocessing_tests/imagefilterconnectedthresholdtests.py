"""
PyZinc Unit Tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

'''
Created on October 11, 2013

@author: Alan Wu
'''
import unittest

from opencmiss.zinc.context import Context

class ImagefilterConnectedThresholdTestsCase(unittest.TestCase):

    def setUp(self):
        self.context = Context("ImagefilterConnectedThresholdTest")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldmodule()

    def tearDown(self):
        del self.field_module
        del self.context

    def testFieldImagefilterConnectedThresholdCreate(self):
        self.assertRaises(TypeError, self.field_module.createFieldImagefilterConnectedThreshold, [1])
        f1 = self.field_module.createFieldConstant([2])        
        f = self.field_module.createFieldImagefilterConnectedThreshold(f1, 0.2, 1.0, 1.0, 1, [ 0.3, 0.1, 0.7 ])
        self.assertFalse(f.isValid())

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImagefilterConnectedThresholdTestsCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
