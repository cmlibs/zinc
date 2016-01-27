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
from opencmiss.zinc import status

class ImagefilterHistogramTestsCase(unittest.TestCase):

    def setUp(self):
        self.context = Context("ImagefilterHistogramTest")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldmodule()

    def tearDown(self):
        del self.field_module
        del self.context

    def testFieldImagefilterHistogramCreate(self):
        self.assertRaises(TypeError, self.field_module.createFieldImagefilterHistogram, [1])
        imageField = self.field_module.createFieldImage()
        si = imageField.createStreaminformationImage()
        sr = si.createStreamresourceFile('resource/testimage_gray.jpg')
        imageField.read(si);   
        f = self.field_module.createFieldImagefilterHistogram(imageField)
        self.assertTrue(f.isValid())
        result = f.setComputeMaximumValues([2.0])
        self.assertEqual(status.OK, result)
        return_code = f.getComputeMaximumValues(1)
        self.assertEqual(status.OK, return_code[0])
        self.assertEqual(2.0, return_code[1])
        result = f.setComputeMinimumValues([0.5])
        self.assertEqual(status.OK, result)
        return_code = f.getComputeMinimumValues(1)
        self.assertEqual(status.OK, return_code[0])
        self.assertEqual(0.5, return_code[1])
        result = f.setMarginalScale(20.0)
        self.assertEqual(status.OK, result)
        return_code = f.getMarginalScale()
        self.assertEqual(20.0, return_code)
        result = f.setNumberOfBins([20])
        self.assertEqual(status.OK, result)
        return_code = f.getNumberOfBins(1)
        self.assertEqual(status.OK, return_code[0])
        self.assertEqual(20, return_code[1])
        
def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImagefilterHistogramTestsCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
