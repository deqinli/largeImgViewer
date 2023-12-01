#pragma once
#include "Position.h"
#include "MapGraphicsObject.h"

class LineObject : public MapGraphicsObject
{
    Q_OBJECT
public:
    explicit LineObject(const Position& endA,
        const Position& endB,
        qreal thickness = 0.0,
        MapGraphicsObject* parent = 0);
    virtual ~LineObject();

    //pure-virtual from MapGraphicsObject
    QRectF boundingRect() const;

    //pure-virtual from MapGraphicsObject
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    qreal thickness() const;
    void setThickness(qreal nThick);

signals:

public slots:
    void setEndPointA(const Position& pos);
    void setEndPointB(const Position& pos);
    void setEndPoints(const Position& a,
        const Position& b);

private slots:
    void updatePositionFromEndPoints();

private:
    Position _a;
    Position _b;
    qreal _thickness;

};

