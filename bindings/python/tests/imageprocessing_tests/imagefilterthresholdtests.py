"""
PyZinc Unit tests

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
from opencmiss.zinc import status

class ImagefilterThresholdTestsCase(unittest.TestCase):

    def setUp(self):
        self.context = Context("ImagefilterThresholdTest")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldmodule()

    def tearDown(self):
        del self.field_module
        del self.context

    def testFieldImagefilterThresholdCreate(self):
        self.assertRaises(TypeError, self.field_module.createFieldImagefilterThreshold, [1])
        imageField = self.field_module.createFieldImage()
        si = imageField.createStreaminformationImage()
        sr = si.createStreamresourceFile('resource/testimage_gray.jpg')
        imageField.read(si);   
        f = self.field_module.createFieldImagefilterThreshold(imageField)
        self.assertTrue(f.isValid())
        condition = f.getCondition();
        self.assertEqual(f.CONDITION_BELOW, condition)
        result = f.setCondition(f.CONDITION_OUTSIDE)
        self.assertEqual(status.OK, result)
        condition = f.getCondition();
        self.assertEqual(f.CONDITION_OUTSIDE, condition)
        value = f.getOutsideValue()
        self.assertEqual(0.0, value)
        result = f.setOutsideValue(0.5)
        self.assertEqual(status.OK, result)
        value = f.getOutsideValue()
        self.assertEqual(0.5, value)
        value = f.getLowerThreshold()
        self.assertEqual(0.5, value)
        result = f.setLowerThreshold(0.0)
        self.assertEqual(status.OK, result)
        value = f.getLowerThreshold()
        self.assertEqual(0.0, value)
        value = f.getUpperThreshold()
        self.assertEqual(0.5, value)
        result = f.setUpperThreshold(1.0)
        self.assertEqual(status.OK, result)
        value = f.getUpperThreshold()
        self.assertEqual(1.0, value)
        
def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImagefilterThresholdTestsCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
