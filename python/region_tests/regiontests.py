'''
Created on May 22, 2013

@author: hsorby
'''
import unittest

from zinc.context import Context
#from zinc.region import Region

class RegionTestCase(unittest.TestCase):

    def setUp(self):
        self.c = Context("region")
        
    def tearDown(self):
        del self.c
        
    def testRegion(self):
        rr = self.c.getDefaultRegion()
        self.assertTrue(rr.isValid())
        sr = rr.createSubregion("bob/rick/mark")
        self.assertTrue(sr.isValid())
        tr = rr.findSubregionAtPath("/bob/rick/mark")
        self.assertTrue(tr.isValid())


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(RegionTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
