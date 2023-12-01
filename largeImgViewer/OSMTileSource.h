#pragma once
#include "TileSource.h"
#include <QSet>
#include <QHash>

class QNetworkReply;

class OSMTileSource : public TileSource
{
	Q_OBJECT;

public:
	enum OSMTileType
	{
		OSMTiles
	};

public:
	explicit OSMTileSource(OSMTileSource::OSMTileType tileType = OSMTiles);
	virtual ~OSMTileSource();

	virtual QPointF ll2qgs(const QPointF& ll, quint8 zoomLevel) const;
	virtual QPointF qgs2ll(const QPointF& qgs, quint8 zoomLevel) const;
	virtual quint64 tilesNumOnZoomLevel(quint8 zoomLevel) const;
	virtual quint16 getTileSize() const;
	virtual quint8 minZoomLevel(QPointF ll);
	virtual quint8 maxZoomLevel(QPointF ll);
	virtual QString name() const;
	virtual QString tileFileExtension() const;

protected:
	virtual void fetchTile(quint32 x, quint32 y, quint8 z);

private:
	OSMTileSource::OSMTileType _tileType;

	QSet<QString> _pendingRequests;
	QHash<QNetworkReply*, QString> _pendingReplies;

private slots:
	void handleNetworkRequestFinished();
};

