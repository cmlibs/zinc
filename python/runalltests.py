#!/usr/bin/python
import unittest

def suite():
    tests = unittest.TestSuite()

    from import_tests import importtests
    tests.addTests(importtests.suite())
    
    from region_tests import regiontests
    tests.addTests(regiontests.suite())

    from graphic_tests import graphictests
    tests.addTests(graphictests.suite())
    
    from field_tests import fieldtests
    tests.addTests(fieldtests.suite())
    
    from sceneviewer_tests import sceneviewertests
    tests.addTests(sceneviewertests.suite())

    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

