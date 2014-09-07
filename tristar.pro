QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CFLAGS += -std=c99
QMAKE_CXXFLAGS += -std=c++11

TARGET = tristar
TEMPLATE = app
CONFIG += c++11

# OS-specific metadata and stuff
win32:RC_FILE = src/windows.rc

# build on OS X with xcode/clang and libc++
macx:QMAKE_CXXFLAGS += -stdlib=libc++

SOURCES += \
    src/mapscene.cpp \
    src/mainwindow.cpp \
    src/main.cpp \
    src/level.cpp \
    src/objectwindow.cpp
    
HEADERS  += \
    src/mapscene.h \
    src/mainwindow.h \
    src/version.h \
    src/level.h \
    src/objectwindow.h
    
FORMS += \
    src/mainwindow.ui \
    src/objectwindow.ui

RESOURCES += \
    src/icons.qrc

OTHER_FILES += \
    src/windows.rc \
    README.txt
