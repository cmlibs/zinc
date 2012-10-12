#!/usr/bin/python
import unittest

if __name__ == '__main__':
    tests = unittest.TestSuite()

    from import_tests import ImportTests
    tests.addTests(ImportTests.suite())

    unittest.TextTestRunner().run(tests)

