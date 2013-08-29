'''
Created on May 22, 2013

@author: hsorby
'''
import unittest

from zinc.context import Context
from zinc.glyph import Glyph
from zinc.graphic import Graphic
from zinc.field import Field

class GraphicTestCase(unittest.TestCase):


    def setUp(self):
        self.context = Context('graphictest')
        root_region = self.context.getDefaultRegion()
        self.graphics_module = self.context.getGraphicsModule()
        self.scene = self.graphics_module.getScene(root_region)


    def tearDown(self):
        del self.graphics_module
        del self.scene
        del self.context


    def testGraphicCreation(self):
        graphic = self.scene.createGraphicPoints()
        self.assertTrue(graphic.isValid())
        result = graphic.setDomainType(Field.DOMAIN_NODES)
        attributes = graphic.getPointAttributes()
        self.assertTrue(attributes.isValid())
        glyph_module = self.graphics_module.getGlyphModule()
        glyph_module.defineStandardGlyphs()
        result = attributes.setGlyphType(Glyph.SPHERE)
        self.assertEqual(1, result)

    def testGraphicSetBaseSize(self):
        graphic = self.scene.createGraphicPoints()
        self.assertTrue(graphic.isValid())
        attributes = graphic.getPointAttributes()
        result = attributes.setBaseSize([1])
        self.assertEqual(1, result)
        result = attributes.setBaseSize([6, 2, 0.3])
        self.assertEqual(1, result)
        result = attributes.setBaseSize(3)
        self.assertEqual(1, result)

    def testGraphicGetBaseSize(self):
        graphic = self.scene.createGraphicPoints()
        self.assertTrue(graphic.isValid())
        attributes = graphic.getPointAttributes()
        result = attributes.setBaseSize(5.4)
        self.assertEqual(1, result)
        base_size = attributes.getBaseSize(1)
        self.assertEqual(1, base_size[0])
        self.assertEqual(5.4, base_size[1])
        attributes.setBaseSize([4.8, 2.1, 7])
        base_size = attributes.getBaseSize(3)
        self.assertEqual([4.8, 2.1, 7], base_size[1])
    
def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(GraphicTestCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
