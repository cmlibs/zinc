import os, sys, unittest

class ImportTestCase(unittest.TestCase):

    def testImportContext(self):
        from Zn import Context

    def testImportRegion(self):
        from Zn import Region

    def testImportFieldModule(self):
        from Zn import FieldModule
    
    def testImportGraphicsModule(self):
        from Zn import GraphicsModule
    
    def testImportMaterial(self):
        from Zn import Material
    
    def testImportRendition(self):
        from Zn import Rendition
    
    def testImportField(self):
        from Zn import Field
    
    def testImportStream(self):
        from Zn import Stream
    
    def testImportElement(self):
        from Zn import Element
    
    def testImportNode(self):
        from Zn import Node
    
    def testImportDifferentialOperator(self):
        from Zn import DifferentialOperator
    
    def testImportFieldCache(self):
        from Zn import FieldCache
    
    def testImportTessellation(self):
        from Zn import Tessellation
    
    def testImportGraphic(self):
        from Zn import Graphic
    
    def testImportSelection(self):
        from Zn import Selection
    
    def testImportGraphicsFilter(self):
        from Zn import GraphicsFilter
    
    def testImportScene(self):
        from Zn import Scene
    
    def testImportSpectrum(self):
        from Zn import Spectrum
    
    def testImportOptimisation(self):
        from Zn import Optimisation
    
    def testImportTime(self):
        from Zn import Time
    
    def testImportTimeSequence(self):
        from Zn import TimeSequence
    


def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImportTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
