"""
PyZinc Unit tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

import unittest

from opencmiss.zinc.context import Context
from opencmiss.zinc.logger import Logger, Loggernotifier, Loggerevent
#from opencmiss.zinc.region import Region

def loggerCallback(event):
    global lastloggerevent 
    lastloggerevent = event

class LoggerTestCase(unittest.TestCase):

    def setUp(self):
        self.c = Context("logger")
        self.root_region = self.c.getDefaultRegion()
        self.logger = self.c.getLogger()
        
    def tearDown(self):
        del self.c
               
    def testLogger(self):
        loggernotifier = self.logger.createLoggernotifier()
        loggernotifier.setCallback(loggerCallback)
        self.root_region.readFile('resource/region_incorrect.exregion')
        self.assertEqual(lastloggerevent.getMessageType(), Logger.MESSAGE_TYPE_ERROR)
        self.assertTrue(lastloggerevent.getLogger().isValid())
        numberOfMessages = self.logger.getNumberOfMessages()
        self.assertEqual(numberOfMessages, 2)

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(LoggerTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
