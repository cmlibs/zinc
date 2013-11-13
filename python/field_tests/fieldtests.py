'''
Created on May 23, 2013

@author: hsorby
'''
import unittest

from field_tests import compositetests, sceneviewerprojectionfieldtests, vectoroperatortests

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(compositetests.suite())
    tests.addTests(sceneviewerprojectionfieldtests.suite())
    tests.addTests(vectoroperatortests.suite())
    
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

