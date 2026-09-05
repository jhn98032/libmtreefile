#
# Written by Johan Henriksson. Copyright (C) 2024-2025.
#
from sys import platform
from build import FORCE_QT5,AUTODETECT

#-----------------------------#
# Configuration section
g_parallel_builds = 4
g_dest_path = "/work/dist"
g_verbose = False
g_debugBuild = False
g_exeName = "viewer"
g_qtVersionToUse = FORCE_QT5
g_qmakeQt4 = ""
g_qmakeQt5 = ""
g_qmakeQt6 = ""
g_testDirs = [
            ]
g_mainSrcDir = ["./src" ]
g_requiredPrograms = ["make", "gcc"  ]
MIN_QT_VER = "5.0.0"
#-----------------------------#
