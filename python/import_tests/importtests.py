import unittest

class ImportTestCase(unittest.TestCase):

    def testImportContext(self):
        from opencmiss.zinc import context

    def testImportDifferentialOperator(self):
        from opencmiss.zinc import differentialoperator
    
    def testImportElement(self):
        from opencmiss.zinc import element
    
    def testImportField(self):
        from opencmiss.zinc import field
    
    def testImportFieldCache(self):
        from opencmiss.zinc import fieldcache
    
    def testImportFieldmodule(self):
        from opencmiss.zinc import fieldmodule
    
    def testImportGraphics(self):
        from opencmiss.zinc import graphics
    
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
    
    def testImportSceneCoordinateSystem(self):
        from opencmiss.zinc import scenecoordinatesystem
        
    def testImportSceneFilter(self):
        from opencmiss.zinc import scenefilter
    
    def testImportSceneViewer(self):
        from opencmiss.zinc import sceneviewer
    
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
    
    def testImportTimeKeeper(self):
        from opencmiss.zinc import timekeeper
    
    def testImportTimeNotifier(self):
        from opencmiss.zinc import timenotifier
    
    def testImportTimeSequence(self):
        from opencmiss.zinc import timesequence
    


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImportTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
