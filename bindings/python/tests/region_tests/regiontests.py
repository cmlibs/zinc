"""
PyZinc Unit tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

'''
Created on May 22, 2013

@author: hsorby, rchristie
'''
import unittest

from opencmiss.zinc.context import Context
from opencmiss.zinc.field import Field
#from opencmiss.zinc.region import Region
from opencmiss.zinc.status import OK as ZINC_OK

class RegionTestCase(unittest.TestCase):

    def setUp(self):
        self.c = Context("region")
        
    def tearDown(self):
        del self.c
        
    def findRegion(self, r, path):
        r.findSubregionAtPath(path)
        self.assertTrue(r.isValid())
        
    def testRegion(self):
        rr = self.c.getDefaultRegion()
        self.assertTrue(rr.isValid())
        sr = rr.createSubregion("bob/rick/mark")
        self.assertTrue(sr.isValid())
        tr = rr.findSubregionAtPath("/bob/rick/mark")
        self.assertTrue(tr.isValid())
        self.findRegion(rr, '/bob/rick/mark')

    def testWriteFieldNames(self):
        """
        This also tests SWIG typemaps for passing strings and lists of string
        """
        rr = self.c.getDefaultRegion()
        self.assertTrue(rr.isValid())
        fm = rr.getFieldmodule()
        self.assertTrue(fm.isValid())
        bob = fm.createFieldFiniteElement(1)
        self.assertTrue(bob.isValid())
        # following calls test SWIG wrappers for setting/getting single strings
        self.assertEquals(ZINC_OK, bob.setName("bob"))
        self.assertEquals("bob", bob.getName())
        fred = fm.createFieldFiniteElement(1)
        self.assertTrue(fred.isValid())
        self.assertEquals(ZINC_OK, fred.setName("fred"))
        joe = fm.createFieldFiniteElement(1)
        self.assertTrue(joe.isValid())
        self.assertEquals(ZINC_OK, joe.setName("joe"))
        nodes = fm.findNodesetByFieldDomainType(Field.DOMAIN_TYPE_NODES)
        nt = nodes.createNodetemplate()
        self.assertEquals(ZINC_OK, nt.defineField(bob))
        self.assertEquals(ZINC_OK, nt.defineField(fred))
        self.assertEquals(ZINC_OK, nt.defineField(joe))
        node = nodes.createNode(1, nt)
        self.assertTrue(node.isValid())

        sir = rr.createStreaminformationRegion()
        self.assertTrue(sir.isValid())
        mr = sir.createStreamresourceMemory()
        try:
            sir.setResourceFieldNames(mr, 5)
            self.fail("setResourceFieldNames with integer argument did not raise exception")
        except TypeError:
            pass
        try:
            sir.setResourceFieldNames(mr, [5, "joe"])
            self.fail("setResourceFieldNames with integer list member did not raise exception")
        except TypeError:
            pass
        sir.setResourceFieldNames(mr, ["bob", "joe"])
        result = rr.write(sir)
        self.assertEqual(ZINC_OK, result)
        result, output = mr.getBuffer()
        self.assertEqual(ZINC_OK, result)
        self.assertTrue("bob" in output)
        self.assertFalse("fred" in output)
        self.assertTrue("joe" in output)

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(RegionTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
