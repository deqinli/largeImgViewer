#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "MapGraphicsView.h"
#include "MapGraphicsScene.h"
#include "OSMTileSource.h"
#include "CompositeTileSource.h"
#include "CompositeTileSourceWidget.h"
#include "PolygonObject.h"
#include <QSharedPointer>
#include <QDebug>
#include <QThread>
#include <QImage>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Setup the MapGraphics scene and view
    MapGraphicsScene* scene = new MapGraphicsScene(this);
    MapGraphicsView* view = new MapGraphicsView(scene, this);

    //The view will be our central widget
    this->setCentralWidget(view);
    this->ui->dockWidget->setVisible(false);

    //Setup some tile sources
    QSharedPointer<OSMTileSource> osmTiles(new OSMTileSource(OSMTileSource::OSMTiles), &QObject::deleteLater);
    //QSharedPointer<GridTileSource> gridTiles(new GridTileSource(), &QObject::deleteLater);
    QSharedPointer<CompositeTileSource> composite(new CompositeTileSource(), &QObject::deleteLater);
    composite->addSourceBottom(osmTiles);
    //composite->addSourceTop(gridTiles);
    view->setTileSource(composite);

    //Create a widget in the dock that lets us configure tile source layers
    //CompositeTileSourceWidget* tileConfigWidget = new CompositeTileSourceWidget(composite.toWeakRef(),this->ui->dockWidget);
    //this->ui->dockWidget->setWidget(tileConfigWidget);
    //delete this->ui->dockWidgetContents;

    //this->ui->menuWindow->addAction(this->ui->dockWidget->toggleViewAction());
    //this->ui->dockWidget->toggleViewAction()->setText("&Layers");

    view->setZoomLevel(0);
    view->centerOn(-112.202442, 40.9936234);

    // Create a circle on the map to demonstrate MapGraphicsObject a bit
    // The circle can be clicked/dragged around and should be ~5km in radius
    //MapGraphicsObject* circle = new CircleObject(5000, false, QColor(255, 0, 0, 100));
    //circle->setLatitude(40.9936234);
    //circle->setLongitude(-112.202442);
    //scene->addObject(circle);

}

MainWindow::~MainWindow()
{
    delete ui;
}

