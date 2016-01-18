"""
PyZinc Unit Tests

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
"""

import unittest

class ImportTestCase(unittest.TestCase):

    def testImportContext(self):
        from opencmiss.zinc import context

    def testImportDifferentialoperator(self):
        from opencmiss.zinc import differentialoperator
    
    def testImportElement(self):
        from opencmiss.zinc import element
    
    def testImportField(self):
        from opencmiss.zinc import field
    
    def testImportFieldcache(self):
        from opencmiss.zinc import fieldcache
    
    def testImportFieldmodule(self):
        from opencmiss.zinc import fieldmodule
    
    def testImportGraphics(self):
        from opencmiss.zinc import graphics
        
    def testImportLight(self):
        from opencmiss.zinc import light
        
    def testImportLogger(self):
        from opencmiss.zinc import logger
    
    def testImportMaterial(self):
        from opencmiss.zinc import material
    
    def testImportNode(self):
        from opencmiss.zinc import node
    
    def testImportOptimisation(self):
        from opencmiss.zinc import optimisation
    
    def testImportRegion(self):
        from opencmiss.zinc import region
    
    def testImportScene(self):
        from opencmiss.zinc import scene
    
    def testImportScenecoordinatesystem(self):
        from opencmiss.zinc import scenecoordinatesystem
        
    def testImportScenefilter(self):
        from opencmiss.zinc import scenefilter
    
    def testImportSceneviewer(self):
        from opencmiss.zinc import sceneviewer
    
    def testImportSceneviewerinput(self):
        from opencmiss.zinc import sceneviewerinput
    
    def testImportSelection(self):
        from opencmiss.zinc import selection
    
    def testImportSpectrum(self):
        from opencmiss.zinc import spectrum
    
    def testImportStatus(self):
        from opencmiss.zinc import status
    
    def testImportStream(self):
        from opencmiss.zinc import stream
    
    def testImportTessellation(self):
        from opencmiss.zinc import tessellation
    
    def testImportTimekeeper(self):
        from opencmiss.zinc import timekeeper
    
    def testImportTimenotifier(self):
        from opencmiss.zinc import timenotifier
    
    def testImportTimesequence(self):
        from opencmiss.zinc import timesequence
    


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImportTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
