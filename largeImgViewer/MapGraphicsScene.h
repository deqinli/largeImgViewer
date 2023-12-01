#pragma once

#include "MapGraphicsObject.h"

#include <QObject>
#include <QList>
#include <QSet>

class MapGraphicsScene : public QObject
{
	Q_OBJECT;
public:
    MapGraphicsScene(QObject* parent = 0);
    virtual ~MapGraphicsScene();

    void addObject(MapGraphicsObject* object);

    QList<MapGraphicsObject*> objects() const;

    void removeObject(MapGraphicsObject* object);

signals:
   
    void objectAdded(MapGraphicsObject*);

    void objectRemoved(MapGraphicsObject*);

private slots:
    void handleNewObjectGenerated(MapGraphicsObject* newObject);
    void handleObjectDestroyed(QObject* object);

private:
    QSet<MapGraphicsObject*> _objects;

};

