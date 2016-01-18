#!/usr/bin/python
"""
PyZinc Unit Tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

import unittest

def suite():
    tests = unittest.TestSuite()

    from import_tests import importtests
    tests.addTests(importtests.suite())
    
    from region_tests import regiontests
    tests.addTests(regiontests.suite())

    from graphics_tests import graphicstests
    tests.addTests(graphicstests.suite())
    
    from field_tests import fieldtests
    tests.addTests(fieldtests.suite())
    
    from logger_tests import loggertests
    tests.addTests(loggertests.suite())
    
    from sceneviewer_tests import sceneviewertests
    tests.addTests(sceneviewertests.suite())
    
    from imageprocessing_tests import imageprocessingtests
    tests.addTests(imageprocessingtests.suite())
    
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())

