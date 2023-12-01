#include "MapGraphicsScene.h"
#include <QtDebug>


MapGraphicsScene::MapGraphicsScene(QObject* parent)
    : QObject(parent)
{
}

MapGraphicsScene::~MapGraphicsScene()
{
    foreach(MapGraphicsObject * obj, _objects)
        this->removeObject(obj);
}

void MapGraphicsScene::addObject(MapGraphicsObject* object)
{
    if (object == 0)
        return;

    connect(object,
        SIGNAL(newObjectGenerated(MapGraphicsObject*)),
        this,
        SLOT(handleNewObjectGenerated(MapGraphicsObject*)));
    connect(object,
        SIGNAL(destroyed(QObject*)),
        this,
        SLOT(handleObjectDestroyed(QObject*)));

    _objects.insert(object);
    this->objectAdded(object);
}

QList<MapGraphicsObject*> MapGraphicsScene::objects() const
{
    QList<MapGraphicsObject*> toRet;

    return toRet;
}

void MapGraphicsScene::removeObject(MapGraphicsObject* object)
{
    _objects.remove(object);
    this->objectRemoved(object);
}

//private slot
void MapGraphicsScene::handleNewObjectGenerated(MapGraphicsObject* newObject)
{
    this->addObject(newObject);
}

//private slot
void MapGraphicsScene::handleObjectDestroyed(QObject* object)
{
    MapGraphicsObject* mgObj = (MapGraphicsObject*)object;

    this->removeObject(mgObj);
}
