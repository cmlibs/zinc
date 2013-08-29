import os, sys, unittest

class ImportTestCase(unittest.TestCase):

    def testImportContext(self):
        from zinc import context

    def testImportDifferentialOperator(self):
        from zinc import differentialoperator
    
    def testImportElement(self):
        from zinc import element
    
    def testImportField(self):
        from zinc import field
    
    def testImportFieldCache(self):
        from zinc import fieldcache
    
    def testImportFieldModule(self):
        from zinc import fieldmodule
    
    def testImportGraphic(self):
        from zinc import graphic
    
    def testImportGraphicsFilter(self):
        from zinc import graphicsfilter
    
    def testImportGraphicsMaterial(self):
        from zinc import graphicsmaterial
    
    def testImportGraphicsModule(self):
        from zinc import graphicsmodule
    
    def testImportNode(self):
        from zinc import node
    
    def testImportOptimisation(self):
        from zinc import optimisation
    
    def testImportRegion(self):
        from zinc import region
    
    def testImportScene(self):
        from zinc import scene
    
    def testImportSceneCoordinateSystem(self):
        from zinc import scenecoordinatesystem
    
    def testImportSceneViewer(self):
        from zinc import sceneviewer
    
    def testImportSelection(self):
        from zinc import selection
    
    def testImportSpectrum(self):
        from zinc import spectrum
    
    def testImportStatus(self):
        from zinc import status
    
    def testImportStream(self):
        from zinc import stream
    
    def testImportTessellation(self):
        from zinc import tessellation
    
    def testImportTimeKeeper(self):
        from zinc import timekeeper
    
    def testImportTimeNotifier(self):
        from zinc import timenotifier
    
    def testImportTimeSequence(self):
        from zinc import timesequence
    


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImportTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
