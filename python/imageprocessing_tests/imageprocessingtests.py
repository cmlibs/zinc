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

