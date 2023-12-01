#pragma once
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsItem>
#include <QString>


class MapGraphicsObject : public QObject
{
    Q_OBJECT

public:
    enum MapGraphicsObjectFlag
    {
        ObjectIsMovable = 0x01,
        ObjectIsSelectable = 0x02,
        ObjectIsFocusable = 0x04
    };
    Q_DECLARE_FLAGS(MapGraphicsObjectFlags, MapGraphicsObjectFlag)

        friend class PrivateQGraphicsObject;
public:
    explicit MapGraphicsObject(bool sizeIsZoomInvariant = false, MapGraphicsObject* parent = 0);
    virtual ~MapGraphicsObject();

    bool sizeIsZoomInvariant() const;

    virtual QRectF boundingRect() const = 0;

    virtual bool contains(const QPointF& geoPos) const;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) = 0;

    bool enabled() const;
    void setEnabled(bool);

    qreal opacity() const;
    void setOpacity(qreal);

    MapGraphicsObject* parent() const;
    void setParent(MapGraphicsObject*);

    const QPointF& pos() const;
    virtual void setPos(const QPointF&);

    qreal rotation() const;
    void setRotation(qreal);

    bool visible() const;
    void setVisible(bool);

    qreal longitude() const;
    void setLongitude(qreal);

    qreal latitude() const;
    void setLatitude(qreal);

    qreal zValue() const;
    void setZValue(qreal);

    bool isSelected() const;
    void setSelected(bool);

    QString toolTip() const;
    void setToolTip(const QString& toolTip);

    void setFlag(MapGraphicsObjectFlag, bool enabled = true);
    void setFlags(MapGraphicsObject::MapGraphicsObjectFlags);
    MapGraphicsObject::MapGraphicsObjectFlags flags() const;

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    virtual void wheelEvent(QGraphicsSceneWheelEvent* event);


signals:
    void enabledChanged();
    void opacityChanged();
    void parentChanged();
    void posChanged();
    void rotationChanged();
    void visibleChanged();
    void zValueChanged();
    void toolTipChanged(const QString& toolTip);

    void flagsChanged();

    void selectedChanged();

    void newObjectGenerated(MapGraphicsObject*);

    void redrawRequested();

    void keyFocusRequested();


public slots:

private slots:
    void setConstructed();

private:
    bool _sizeIsZoomInvariant;

    bool _enabled;
    qreal _opacity;
    MapGraphicsObject* _parent;
    QPointF _pos;
    qreal _rotation;
    bool _visible;
    qreal _zValue;
    bool _selected;

    QString _toolTip;

    MapGraphicsObject::MapGraphicsObjectFlags _flags;

    bool _constructed;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MapGraphicsObject::MapGraphicsObjectFlags)