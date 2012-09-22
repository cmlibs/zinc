'''
MAP Client, a program to generate detailed musculoskeletal models for OpenSim.
    Copyright (C) 2012  University of Auckland
    
This file is part of MAP Client. (http://launchpad.net/mapclient)

    MAP Client is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MAP Client is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MAP Client.  If not, see <http://www.gnu.org/licenses/>..
'''
import os, sys, unittest
from core.PluginFramework import loadPlugins
from Utils import ConsumeOutput

class PluginFrameworkTestCase(unittest.TestCase):


    def testLoadPlugins(self):
        fileDir = os.path.dirname(__file__)
        inbuiltPluginDir = os.path.realpath(fileDir + '/../../plugins')

        old_stdout = sys.stdout
        sys.stdout = redirectstdout = ConsumeOutput()
        sys.path.insert(0, inbuiltPluginDir)
        loadPlugins(inbuiltPluginDir)

        sys.stdout = old_stdout
        assert("Plugin 'workspace' version 0.2.0 by Hugh Sorby loaded" in redirectstdout.messages)


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testLoadPlugins']
    unittest.main()
