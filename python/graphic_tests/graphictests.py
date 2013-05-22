'''
Created on May 22, 2013

@author: hsorby
'''
import unittest

from zinc.context import Context
from zinc.graphic import Graphic

class GraphicTestCase(unittest.TestCase):


    def setUp(self):
        self.context = Context('graphictest')
        root_region = self.context.getDefaultRegion()
        graphics_module = self.context.getDefaultGraphicsModule()
        graphics_module.enableRenditions(root_region)
        self.rendition = graphics_module.getRendition(root_region)


    def tearDown(self):
        del self.rendition
        del self.context


    def testGraphicCreation(self):
        graphic = self.rendition.createGraphic(Graphic.GRAPHIC_POINT)
        self.assertTrue(graphic.isValid())
        attributes = graphic.getPointAttributes()
        self.assertTrue(attributes.isValid())
        result = attributes.setGlyphType(Graphic.GLYPH_TYPE_AXES)
        self.assertEqual(1, result)

    def testGraphicSetBaseSize(self):
        graphic = self.rendition.createGraphic(Graphic.GRAPHIC_POINT)
        self.assertTrue(graphic.isValid())
        attributes = graphic.getPointAttributes()
        result = attributes.setBaseSize([1])
        self.assertEqual(1, result)
        result = attributes.setBaseSize([6, 2, 0.3])
        self.assertEqual(1, result)
        result = attributes.setBaseSize(3)
        self.assertEqual(1, result)

    def testGraphicGetBaseSize(self):
        graphic = self.rendition.createGraphic(Graphic.GRAPHIC_POINT)
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
