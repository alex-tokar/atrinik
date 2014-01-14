# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '..\ui\ui_window_main.ui'
#
# Created: Tue Nov 12 16:04:55 2013
#      by: PyQt5 UI code generator 5.1.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_WindowMain(object):
    def setupUi(self, WindowMain):
        WindowMain.setObjectName("WindowMain")
        WindowMain.resize(745, 547)
        self.centralwidget = QtWidgets.QWidget(WindowMain)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName("gridLayout")
        self.gridLayout_2 = QtWidgets.QGridLayout()
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.buttonScan = QtWidgets.QPushButton(self.centralwidget)
        self.buttonScan.setStyleSheet("")
        self.buttonScan.setObjectName("buttonScan")
        self.gridLayout_2.addWidget(self.buttonScan, 1, 1, 1, 1)
        self.buttonOpen_all = QtWidgets.QPushButton(self.centralwidget)
        self.buttonOpen_all.setObjectName("buttonOpen_all")
        self.gridLayout_2.addWidget(self.buttonOpen_all, 1, 2, 1, 1)
        spacerItem = QtWidgets.QSpacerItem(40, 0, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_2.addItem(spacerItem, 0, 0, 1, 1)
        self.progressBar = QtWidgets.QProgressBar(self.centralwidget)
        self.progressBar.setStatusTip("")
        self.progressBar.setStyleSheet("")
        self.progressBar.setMaximum(1000000)
        self.progressBar.setProperty("value", 0)
        self.progressBar.setTextVisible(True)
        self.progressBar.setOrientation(QtCore.Qt.Horizontal)
        self.progressBar.setInvertedAppearance(False)
        self.progressBar.setObjectName("progressBar")
        self.gridLayout_2.addWidget(self.progressBar, 1, 0, 1, 1)
        self.gridLayout.addLayout(self.gridLayout_2, 1, 1, 1, 1)
        self.widgetTabs = QtWidgets.QTabWidget(self.centralwidget)
        self.widgetTabs.setObjectName("widgetTabs")
        self.tab = QtWidgets.QWidget()
        self.tab.setObjectName("tab")
        self.gridLayout_4 = QtWidgets.QGridLayout(self.tab)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.widgetTableMaps = QtWidgets.QTableWidget(self.tab)
        self.widgetTableMaps.setObjectName("widgetTableMaps")
        self.widgetTableMaps.setColumnCount(3)
        self.widgetTableMaps.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.widgetTableMaps.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        self.widgetTableMaps.setHorizontalHeaderItem(1, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.widgetTableMaps.setHorizontalHeaderItem(2, item)
        self.widgetTableMaps.horizontalHeader().setStretchLastSection(True)
        self.widgetTableMaps.verticalHeader().setSortIndicatorShown(False)
        self.gridLayout_4.addWidget(self.widgetTableMaps, 0, 0, 1, 1)
        self.widgetTabs.addTab(self.tab, "")
        self.tab_2 = QtWidgets.QWidget()
        self.tab_2.setObjectName("tab_2")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.tab_2)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.widgetTableResources = QtWidgets.QTableWidget(self.tab_2)
        self.widgetTableResources.setObjectName("widgetTableResources")
        self.widgetTableResources.setColumnCount(3)
        self.widgetTableResources.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.widgetTableResources.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignHCenter|QtCore.Qt.AlignVCenter|QtCore.Qt.AlignCenter)
        self.widgetTableResources.setHorizontalHeaderItem(1, item)
        item = QtWidgets.QTableWidgetItem()
        item.setTextAlignment(QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.widgetTableResources.setHorizontalHeaderItem(2, item)
        self.widgetTableResources.horizontalHeader().setStretchLastSection(True)
        self.gridLayout_3.addWidget(self.widgetTableResources, 1, 0, 1, 1)
        self.widgetTabs.addTab(self.tab_2, "")
        self.gridLayout.addWidget(self.widgetTabs, 0, 1, 1, 1)
        WindowMain.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(WindowMain)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 745, 26))
        self.menubar.setObjectName("menubar")
        self.menuFile = QtWidgets.QMenu(self.menubar)
        self.menuFile.setObjectName("menuFile")
        self.menuHelp = QtWidgets.QMenu(self.menubar)
        self.menuHelp.setObjectName("menuHelp")
        self.menuEdit = QtWidgets.QMenu(self.menubar)
        self.menuEdit.setObjectName("menuEdit")
        self.menuNavigate = QtWidgets.QMenu(self.menubar)
        self.menuNavigate.setObjectName("menuNavigate")
        WindowMain.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(WindowMain)
        self.statusbar.setObjectName("statusbar")
        WindowMain.setStatusBar(self.statusbar)
        self.actionAbout = QtWidgets.QAction(WindowMain)
        self.actionAbout.setObjectName("actionAbout")
        self.actionExit = QtWidgets.QAction(WindowMain)
        self.actionExit.setObjectName("actionExit")
        self.actionScan = QtWidgets.QAction(WindowMain)
        self.actionScan.setObjectName("actionScan")
        self.actionProperties = QtWidgets.QAction(WindowMain)
        self.actionProperties.setObjectName("actionProperties")
        self.actionCopy = QtWidgets.QAction(WindowMain)
        self.actionCopy.setObjectName("actionCopy")
        self.actionSelect_all = QtWidgets.QAction(WindowMain)
        self.actionSelect_all.setObjectName("actionSelect_all")
        self.actionOpen_selected_in_editor = QtWidgets.QAction(WindowMain)
        self.actionOpen_selected_in_editor.setObjectName("actionOpen_selected_in_editor")
        self.actionOpen_selected_in_client = QtWidgets.QAction(WindowMain)
        self.actionOpen_selected_in_client.setObjectName("actionOpen_selected_in_client")
        self.actionPreferences = QtWidgets.QAction(WindowMain)
        self.actionPreferences.setObjectName("actionPreferences")
        self.actionReport_a_problem = QtWidgets.QAction(WindowMain)
        self.actionReport_a_problem.setObjectName("actionReport_a_problem")
        self.actionScan_directory = QtWidgets.QAction(WindowMain)
        self.actionScan_directory.setObjectName("actionScan_directory")
        self.menuFile.addAction(self.actionScan)
        self.menuFile.addAction(self.actionScan_directory)
        self.menuFile.addSeparator()
        self.menuFile.addAction(self.actionExit)
        self.menuHelp.addAction(self.actionReport_a_problem)
        self.menuHelp.addSeparator()
        self.menuHelp.addAction(self.actionAbout)
        self.menuEdit.addAction(self.actionCopy)
        self.menuEdit.addAction(self.actionSelect_all)
        self.menuEdit.addSeparator()
        self.menuEdit.addAction(self.actionPreferences)
        self.menuNavigate.addAction(self.actionOpen_selected_in_editor)
        self.menuNavigate.addAction(self.actionOpen_selected_in_client)
        self.menubar.addAction(self.menuFile.menuAction())
        self.menubar.addAction(self.menuEdit.menuAction())
        self.menubar.addAction(self.menuNavigate.menuAction())
        self.menubar.addAction(self.menuHelp.menuAction())

        self.retranslateUi(WindowMain)
        self.widgetTabs.setCurrentIndex(0)
        self.buttonScan.clicked.connect(self.actionScan.trigger)
        QtCore.QMetaObject.connectSlotsByName(WindowMain)

    def retranslateUi(self, WindowMain):
        _translate = QtCore.QCoreApplication.translate
        WindowMain.setWindowTitle(_translate("WindowMain", "Map Checker"))
        self.buttonScan.setText(_translate("WindowMain", "Scan"))
        self.buttonOpen_all.setText(_translate("WindowMain", "Open all"))
        self.widgetTableMaps.setSortingEnabled(True)
        item = self.widgetTableMaps.horizontalHeaderItem(0)
        item.setText(_translate("WindowMain", "Map name"))
        item = self.widgetTableMaps.horizontalHeaderItem(1)
        item.setText(_translate("WindowMain", "Error level"))
        item = self.widgetTableMaps.horizontalHeaderItem(2)
        item.setText(_translate("WindowMain", "Description"))
        self.widgetTabs.setTabText(self.widgetTabs.indexOf(self.tab), _translate("WindowMain", "Maps"))
        self.widgetTableResources.setSortingEnabled(True)
        item = self.widgetTableResources.horizontalHeaderItem(0)
        item.setText(_translate("WindowMain", "Resource name"))
        item = self.widgetTableResources.horizontalHeaderItem(1)
        item.setText(_translate("WindowMain", "Error level"))
        item = self.widgetTableResources.horizontalHeaderItem(2)
        item.setText(_translate("WindowMain", "Description"))
        self.widgetTabs.setTabText(self.widgetTabs.indexOf(self.tab_2), _translate("WindowMain", "Resources"))
        self.menuFile.setTitle(_translate("WindowMain", "File"))
        self.menuHelp.setTitle(_translate("WindowMain", "Help"))
        self.menuEdit.setTitle(_translate("WindowMain", "Edit"))
        self.menuNavigate.setTitle(_translate("WindowMain", "Navigate"))
        self.actionAbout.setText(_translate("WindowMain", "About"))
        self.actionExit.setText(_translate("WindowMain", "Exit"))
        self.actionScan.setText(_translate("WindowMain", "Scan"))
        self.actionProperties.setText(_translate("WindowMain", "Properties"))
        self.actionCopy.setText(_translate("WindowMain", "Copy"))
        self.actionSelect_all.setText(_translate("WindowMain", "Select all"))
        self.actionOpen_selected_in_editor.setText(_translate("WindowMain", "Open selected in editor"))
        self.actionOpen_selected_in_client.setText(_translate("WindowMain", "Open selected in client"))
        self.actionPreferences.setText(_translate("WindowMain", "Preferences"))
        self.actionReport_a_problem.setText(_translate("WindowMain", "Report a problem"))
        self.actionScan_directory.setText(_translate("WindowMain", "Scan directory..."))

