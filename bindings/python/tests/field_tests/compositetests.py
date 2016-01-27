"""
PyZinc Unit Tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

'''
Created on May 23, 2013

@author: hsorby
'''
import unittest

from opencmiss.zinc.context import Context

class CompositeTestCase(unittest.TestCase):


    def setUp(self):
        self.context = Context("compositetest")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldmodule()


    def tearDown(self):
        del self.field_module
        del self.context


    def testCompositeFieldCreate(self):
        self.assertRaises(TypeError, self.field_module.createFieldConcatenate, [1])
        f1 = self.field_module.createFieldConstant([2])
        f2 = self.field_module.createFieldConstant([5])
        f = self.field_module.createFieldConcatenate([f1, f2])
        self.assertTrue(f.isValid())

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(CompositeTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
