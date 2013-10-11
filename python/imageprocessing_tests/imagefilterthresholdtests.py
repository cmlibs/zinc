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
        si = imageField.createStreaminformation()
        sr = si.createStreamresourceFile('resource/testimage_gray.jpg')
        imageField.read(si);   
        f = self.field_module.createFieldImagefilterThreshold(imageField)
        self.assertTrue(f.isValid())
        result = f.setMode(f.THRESHOLD_MODE_OUTSIDE)
        self.assertEqual(status.OK, result)
        result = f.setOutsideValue(0.5)
        self.assertEqual(status.OK, result)
        result = f.setBelow(0.0)
        self.assertEqual(status.OK, result)
        result = f.setAbove(1.0)
        self.assertEqual(status.OK, result)
        
def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImagefilterThresholdTestsCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
