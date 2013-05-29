'''
Created on May 23, 2013

@author: hsorby
'''
import unittest

from zinc.context import Context

class VectorOperatorTestCase(unittest.TestCase):


    def setUp(self):
        self.context = Context("vectoroperatorstest")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldModule()

    def tearDown(self):
        del self.field_module
        del self.context

    def testCrossProductCreate(self):
        self.assertRaises(NotImplementedError, self.field_module.createCrossProduct)
        self.assertRaises(TypeError, self.field_module.createCrossProduct, [0])
        f = self.field_module.createConstant([1, 2])
        cp = self.field_module.createCrossProduct([f])
        self.assertTrue(cp.isValid())


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(VectorOperatorTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
