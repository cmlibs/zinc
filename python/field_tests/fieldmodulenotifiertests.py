'''
Created on May 23, 2013

@author: hsorby
'''
import unittest

from opencmiss.zinc.context import Context
from opencmiss.zinc.fieldmodule import Fieldmodule, Fieldmodulenotifier, Fieldmoduleevent
import opencmiss.zinc.status

lastfieldevent = Fieldmoduleevent.CHANGE_FLAG_NONE

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
        
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_ADD, lastfieldevent.getChangeFlags())

        self.assertEqual(opencmiss.zinc.status.OK, joe.setName("joe"))
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_IDENTIFIER, lastfieldevent.getChangeFlags())
        
        cache = self.field_module.createFieldcache()
        self.assertEqual(opencmiss.zinc.status.OK, joe.assignReal(cache, [4.5]))
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_DEFINITION, lastfieldevent.getChangeFlags())
        
        self.assertEqual(opencmiss.zinc.status.OK, joe.setManaged(True));
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_METADATA, lastfieldevent.getChangeFlags())

        fred = self.field_module.createFieldMagnitude(joe)
        self.assertTrue(fred.isValid())
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_ADD, lastfieldevent.getChangeFlags())
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_ADD, lastfieldevent.getFieldChangeFlags(fred))
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_NONE, lastfieldevent.getFieldChangeFlags(joe))

        self.assertEqual(opencmiss.zinc.status.OK, joe.assignReal(cache, [4.5]))
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_DEFINITION | Fieldmoduleevent.CHANGE_FLAG_DEPENDENCY,
            lastfieldevent.getChangeFlags())
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_DEFINITION, lastfieldevent.getFieldChangeFlags(joe))
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_DEPENDENCY, lastfieldevent.getFieldChangeFlags(fred))

        self.assertEqual(opencmiss.zinc.status.OK, joe.setManaged(False))
        joe = 0
        fred = 0
        self.assertEqual(Fieldmoduleevent.CHANGE_FLAG_REMOVE, lastfieldevent.getChangeFlags())

        self.assertEqual(opencmiss.zinc.status.OK, notifier.clearCallback())


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(FieldmodulenotifierTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
