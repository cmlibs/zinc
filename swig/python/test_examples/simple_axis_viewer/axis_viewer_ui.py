# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'axis_viewer.ui'
#
# Created: Tue Sep 25 19:51:26 2012
#      by: PyQt4 UI code generator 4.9.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_AxisViewerDlg(object):
    def setupUi(self, AxisViewerDlg):
        AxisViewerDlg.setObjectName(_fromUtf8("AxisViewerDlg"))
        AxisViewerDlg.resize(400, 300)
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(_fromUtf8(":/cmiss_icon.ico")), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        AxisViewerDlg.setWindowIcon(icon)
        self.gridLayout = QtGui.QGridLayout(AxisViewerDlg)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.pushButton = QtGui.QPushButton(AxisViewerDlg)
        self.pushButton.setObjectName(_fromUtf8("pushButton"))
        self.gridLayout.addWidget(self.pushButton, 1, 1, 1, 1)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem, 1, 0, 1, 1)
        self.widget = QtGraphicsCanvas(AxisViewerDlg)
        self.widget.setObjectName(_fromUtf8("widget"))
        self.gridLayout.addWidget(self.widget, 0, 0, 1, 2)

        self.retranslateUi(AxisViewerDlg)
        QtCore.QObject.connect(self.pushButton, QtCore.SIGNAL(_fromUtf8("clicked()")), AxisViewerDlg.close)
        QtCore.QMetaObject.connectSlotsByName(AxisViewerDlg)

    def retranslateUi(self, AxisViewerDlg):
        AxisViewerDlg.setWindowTitle(QtGui.QApplication.translate("AxisViewerDlg", "Axis Viewer", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButton.setText(QtGui.QApplication.translate("AxisViewerDlg", "&Quit", None, QtGui.QApplication.UnicodeUTF8))

from qtgraphicscanvas import QtGraphicsCanvas
import icons_rc
