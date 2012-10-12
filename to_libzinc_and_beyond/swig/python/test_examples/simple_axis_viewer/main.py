#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
"""

import sys
from PyQt4 import QtGui
from axis_viewer_ui import Ui_AxisViewerDlg

class AxisViewerDlg(QtGui.QWidget):
    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.ui = Ui_AxisViewerDlg()
        self.ui.setupUi(self)
        self.setWindowIcon(QtGui.QIcon("cmiss_icon.ico"))

def main():
    
    app = QtGui.QApplication(sys.argv)

    w = AxisViewerDlg()
    w.show()

    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
