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

from field_tests import compositetests, fieldmodulenotifiertests, sceneviewerprojectionfieldtests, vectoroperatortests

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(compositetests.suite())
    tests.addTests(fieldmodulenotifiertests.suite())
    tests.addTests(sceneviewerprojectionfieldtests.suite())
    tests.addTests(vectoroperatortests.suite())
    
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

