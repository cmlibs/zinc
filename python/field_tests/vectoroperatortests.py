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

class VectorOperatorTestCase(unittest.TestCase):


    def setUp(self):
        self.context = Context("vectoroperatorstest")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldmodule()

    def tearDown(self):
        del self.field_module
        del self.context

    def testCrossProductCreate(self):
        self.assertRaises(NotImplementedError, self.field_module.createFieldCrossProduct)
        self.assertRaises(TypeError, self.field_module.createFieldCrossProduct, [0])
        f = self.field_module.createFieldConstant([1, 2])
        cp = self.field_module.createFieldCrossProduct([f])
        self.assertTrue(cp.isValid())

    def testCrossProductOverload1(self):
        f1 = self.field_module.createFieldConstant([1, 2, 3])
        f2 = self.field_module.createFieldConstant([4, 5, 6])
        cp = self.field_module.createFieldCrossProduct(f1, f2)
        self.assertTrue(cp.isValid())
        
    def testCrossProductOverload2(self):
        f1 = self.field_module.createFieldConstant([1, 2])
        f2 = self.field_module.createFieldConstant([4, 5])
        cp = self.field_module.createFieldCrossProduct(f1, f2)
        self.assertFalse(cp.isValid())
        
def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(VectorOperatorTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
