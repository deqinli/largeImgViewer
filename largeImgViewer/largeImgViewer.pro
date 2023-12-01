QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CircleObject.cpp \
    CompositeTileSource.cpp \
    CompositeTileSourceWidget.cpp \
    Conversions.cpp \
    GraphicsNetwork.cpp \
    GridTileSource.cpp \
    LineObject.cpp \
    MapGraphicsObject.cpp \
    MapGraphicsScene.cpp \
    MapGraphicsView.cpp \
    MapTileGraphicsObject.cpp \
    OSMTileSource.cpp \
    PolygonObject.cpp \
    Position.cpp \
    PrivateQGraphicsInfoSource.cpp \
    PrivateQGraphicsObject.cpp \
    PrivateQGraphicsScene.cpp \
    PrivateQGraphicsView.cpp \
    TileLayerListModel.cpp \
    TileSource.cpp \
    TileSourceDelegate.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CircleObject.h \
    CompositeTileSource.h \
    CompositeTileSourceWidget.h \
    Conversions.h \
    Convertion.h \
    GraphicsNetwork.h \
    GridTileSource.h \
    LineObject.h \
    MapGraphicsObject.h \
    MapGraphicsScene.h \
    MapGraphicsView.h \
    MapTileGraphicsObject.h \
    OSMTileSource.h \
    PolygonObject.h \
    Position.h \
    PrivateQGraphicsInfoSource.h \
    PrivateQGraphicsObject.h \
    PrivateQGraphicsScene.h \
    PrivateQGraphicsView.h \
    TileLayerListModel.h \
    TileSource.h \
    TileSourceDelegate.h \
    mainwindow.h

FORMS += \
    CompositeTileSourceWidget.ui \
    mainwindow.ui

DESTDIR = ../bin

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
