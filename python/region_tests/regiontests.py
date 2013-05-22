'''
Created on May 22, 2013

@author: hsorby
'''
import unittest

from zinc.region import Region

class RegionTestCase(unittest.TestCase):


    def testRegion(self):
        r = Region()
        self.assertTrue(r.isValid())


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(RegionTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
