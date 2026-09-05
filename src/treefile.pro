TEMPLATE = lib          # build a library
CONFIG += staticlib     # make it static (.a or .lib)
CONFIG += c++17         # optional, set C++ standard
CONFIG += warn_on

# Uncomment the following for debug builds
CONFIG += debug
CONFIG += optimize_debug


TARGET = mqtreefile

SOURCES += \
        mqtreefile.cpp \
        file.cpp \
        zipfile.cpp

HEADERS += \
    mqtreefile.h \
    file.h \
    zipfile.h

# DEFINES += TEST_PROGRAM


LIBS+=-lzip

