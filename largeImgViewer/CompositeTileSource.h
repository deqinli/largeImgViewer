#pragma once
#include "TileSource.h"
#include <QList>
#include <QHash>
#include <QSharedPointer>
#include <QMutex>

class CompositeTileSource : public TileSource
{
	Q_OBJECT;
public:
	explicit CompositeTileSource();
	virtual ~CompositeTileSource();

	virtual QPointF ll2qgs(const QPointF& ll, quint8 zoomLevel) const;
	virtual QPointF qgs2ll(const QPointF& qgs, quint8 zoomLevel) const;
	virtual quint64 tilesNumOnZoomLevel(quint8 zoomLevel) const;
	virtual quint16 getTileSize() const;
	virtual quint8 minZoomLevel(QPointF ll);
	virtual quint8 maxZoomLevel(QPointF ll);
	virtual QString name() const;
	virtual QString tileFileExtension() const;

	void addSourceTop(QSharedPointer<TileSource>, qreal opacity = 1.0);
	void addSourceBottom(QSharedPointer<TileSource>, qreal opacity = 1.0);
	void moveSource(int from, int to);
	void removeSource(int index);
	int numSources() const;
	QSharedPointer<TileSource> getSource(int index) const;
	qreal getOpacity(int index)const;
	void setOpacity(int index, qreal opacity);
	bool getEnabledFlag(int index) const;
	void setEnabledFlag(int index, bool isEnabled);

protected:
	virtual void fetchTile(quint32 x, quint32 y, quint8 z);

signals:
	void sourcesChanged();
	void sourceAdded(int index);
	void sourceRemoved(int index);
	void sourceReordered();

private slots:
	void handleTileRetriecved(quint32 x, quint32 y, quint8 z);
	void clearPendingTiles();

private:
	void doChildThreading(QSharedPointer<TileSource>);
	QMutex* _globalMutex;
	QList<QSharedPointer<TileSource>> _childSources;
	QList<qreal> _childOpacities;
	QList<bool> _childEnabledFlags;

	QHash<QString, QMap<quint32, QImage*>*> _pendingTiles;
};

