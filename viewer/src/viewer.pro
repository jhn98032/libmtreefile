
TEMPLATE = app

SOURCES += example.cpp \
    ../../src/mtreefile.cpp \
    ../../src/mqtreefile.cpp \
    ../../src/zipfile.cpp \
    ../../src/file.cpp

FORMS += mainwindow.ui
SOURCES += mainwindow.cpp
HEADERS += mainwindow.h \
    ../../src/mtreefile.h \
    ../../src/mqtreefile.h \
    ../../src/zipfile.h \
    ../../src/file.h \
    config.h

FORMS+=aboutdialog.ui
SOURCES+=aboutdialog.cpp
HEADERS+=aboutdialog.h



SOURCES+=log.cpp qtutil.cpp util.cpp
HEADERS+=log.h qtutil.h util.h

TARGET=mtreefileviewer
############################################################################




CONFIG += warn_on
CONFIG += debug


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Uncomment the following for debug builds
#QMAKE_CXXFLAGS +=  -g -O0
#CONFIG+= debug


QMAKE_CXXFLAGS += -I./  
QMAKE_CXXFLAGS += -std=c++11 -pedantic -Wno-variadic-macros

equals(QT_MAJOR_VERSION, 6) {
    QT += gui core widgets
    CONFIG += c++17
}
equals(QT_MAJOR_VERSION, 5) {
    QT += gui core widgets
    CONFIG += c++17
}
equals(QT_MAJOR_VERSION, 4) {
    QT += gui core
    CONFIG += c++11
}

# Link to MQtUtil library
QMAKE_CXXFLAGS+=`libmqtutil-config --cflags`  
LIBS+=`libmqtutil-config --ldflags`

LIBS+=-lzip


RESOURCES += \
    resources.qrc

