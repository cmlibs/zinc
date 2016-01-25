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
from opencmiss.zinc.fieldmodule import Fieldmodule, Fieldmodulenotifier, Fieldmoduleevent
from opencmiss.zinc.field import Field
import opencmiss.zinc.status

lastfieldevent = Field.CHANGE_FLAG_NONE

def fieldmoduleCallback(event):
    global lastfieldevent 
    lastfieldevent = event


class FieldmodulenotifierTestCase(unittest.TestCase):


    def setUp(self):
        self.context = Context("fieldmodulenotifier")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldmodule()


    def tearDown(self):
        del self.field_module
        del self.context


    def testFieldmodulenotifierFieldCreate(self):
        self.assertRaises(TypeError, self.field_module.createFieldmodulenotifier, [1])
        
        notifier = self.field_module.createFieldmodulenotifier()
        self.assertTrue(notifier.isValid())
        
        notifier.setCallback(fieldmoduleCallback)
        joe = self.field_module.createFieldConstant([2.0])
        self.assertTrue(joe.isValid())
        
        self.assertEqual(Field.CHANGE_FLAG_ADD, lastfieldevent.getSummaryFieldChangeFlags())

        self.assertEqual(opencmiss.zinc.status.OK, joe.setName("joe"))
        self.assertEqual(Field.CHANGE_FLAG_IDENTIFIER, lastfieldevent.getSummaryFieldChangeFlags())
        
        cache = self.field_module.createFieldcache()
        self.assertEqual(opencmiss.zinc.status.OK, joe.assignReal(cache, [4.5]))
        self.assertEqual(Field.CHANGE_FLAG_DEFINITION | Field.CHANGE_FLAG_FULL_RESULT, lastfieldevent.getSummaryFieldChangeFlags())
        
        self.assertEqual(opencmiss.zinc.status.OK, joe.setManaged(True));
        self.assertEqual(Field.CHANGE_FLAG_DEFINITION, lastfieldevent.getSummaryFieldChangeFlags())

        fred = self.field_module.createFieldMagnitude(joe)
        self.assertTrue(fred.isValid())
        self.assertEqual(Field.CHANGE_FLAG_ADD, lastfieldevent.getSummaryFieldChangeFlags())
        self.assertEqual(Field.CHANGE_FLAG_ADD, lastfieldevent.getFieldChangeFlags(fred))
        self.assertEqual(Field.CHANGE_FLAG_NONE, lastfieldevent.getFieldChangeFlags(joe))

        self.assertEqual(opencmiss.zinc.status.OK, joe.assignReal(cache, [4.5]))
        self.assertEqual(Field.CHANGE_FLAG_DEFINITION | Field.CHANGE_FLAG_FULL_RESULT,
            lastfieldevent.getSummaryFieldChangeFlags())
        self.assertEqual(Field.CHANGE_FLAG_DEFINITION | Field.CHANGE_FLAG_FULL_RESULT, lastfieldevent.getFieldChangeFlags(joe))
        self.assertEqual(Field.CHANGE_FLAG_FULL_RESULT, lastfieldevent.getFieldChangeFlags(fred))

        self.assertEqual(opencmiss.zinc.status.OK, joe.setManaged(False))
        joe = None
        fred = None
        self.assertEqual(Field.CHANGE_FLAG_REMOVE, lastfieldevent.getSummaryFieldChangeFlags())

        self.assertEqual(opencmiss.zinc.status.OK, notifier.clearCallback())


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(FieldmodulenotifierTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
