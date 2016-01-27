"""
PyZinc Unit tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

'''
Created on October 11, 2013

@author: Alan Wu
'''
import unittest

from imageprocessing_tests import imagefilterbinarythresholdtests, imagefilterconnectedthresholdtests, imagefilterhistogramtests, imagefiltermeantests, imagefilterthresholdtests, imagefilterdiscretegaussiantests

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(imagefilterbinarythresholdtests.suite())
    tests.addTests(imagefilterconnectedthresholdtests.suite())
    tests.addTests(imagefilterdiscretegaussiantests.suite())
    tests.addTests(imagefilterhistogramtests.suite())
    tests.addTests(imagefiltermeantests.suite())
    tests.addTests(imagefilterthresholdtests.suite())
    
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

