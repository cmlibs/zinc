#!/usr/bin/python
import unittest

if __name__ == '__main__':
    tests = unittest.TestSuite()

    from import_tests import importtests
    tests.addTests(importtests.suite())

    unittest.TextTestRunner().run(tests)

