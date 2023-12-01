#include "OSMTileSource.h"
#include "GraphicsNetwork.h"

#include <cmath>
#include <QPainter>
#include <QStringBuilder>
#include <QDebug>
#include <QNetworkReply>

const qreal PI = 3.14159265358979323846;
const qreal deg2rad = PI / 180.0;
const qreal rad2deg = 180.0 / PI;

OSMTileSource::OSMTileSource(OSMTileType tileType) : TileSource(), _tileType(tileType)
{
	this->setCacheMode(TileSource::DiskAndMemCaching);
}

OSMTileSource::~OSMTileSource()
{
	qDebug() << this << this->name() << "Destructing";
}

QPointF OSMTileSource::ll2qgs(const QPointF& ll, quint8 zoomLevel) const
{
	const qreal tileOnOneEdge = pow(2.0, zoomLevel);
	const quint16 tileSize = this->getTileSize();
	qreal x = (ll.x() + 180) * (tileOnOneEdge * tileSize) / 360;
	qreal y = (1 - (log(tan(PI / 4 + (ll.y() * deg2rad) / 2)) / PI)) / 2 * (tileOnOneEdge * tileSize);

	return QPointF(int(x),int(y));
}

QPointF OSMTileSource::qgs2ll(const QPointF& qgs, quint8 zoomLevel) const
{
	const qreal tilesOnOneEdge = pow(2.0, zoomLevel);
	const quint16 tileSize = this->getTileSize();
	qreal longitude = (qgs.x() * (360 / (tilesOnOneEdge * tileSize))) - 180;
	qreal latitude = rad2deg * (atan(sinh((1 - qgs.y() * (2 / (tilesOnOneEdge * tileSize))) * PI)));

	return QPointF(longitude, latitude);
}

quint64 OSMTileSource::tilesNumOnZoomLevel(quint8 zoomLevel) const
{
	return pow(4.0, zoomLevel);
}

quint16 OSMTileSource::getTileSize() const
{
	return 256;
}
quint8 OSMTileSource::minZoomLevel(QPointF ll)
{
	Q_UNUSED(ll);
	return 0;
}

quint8 OSMTileSource::maxZoomLevel(QPointF ll)
{
	Q_UNUSED(ll);
	return 18;
}

QString OSMTileSource::name() const
{
	switch (_tileType)
	{
	case OSMTileSource::OSMTiles:
		return "OpenStreetMap Tiles";
		break;
	default:
		return "Unknown Tiles";
		break;
	}
}

QString OSMTileSource::tileFileExtension() const
{
	if (_tileType == OSMTiles)
	{
		return "png";
	}
	else
	{
		return "jpg";
	}
}

void OSMTileSource::fetchTile(quint32 x, quint32 y, quint8 z)
{
	GraphicsNetwork* network = GraphicsNetwork::getInstance();

	QString host;
	QString url;

	//Figure out which server to request from based on our desired tile type
	if (_tileType == OSMTiles)
	{
		host = "https://b.tile.openstreetmap.org";
		url = "/%1/%2/%3.png";
	}

	//Use the unique cacheID to see if this tile has already been requested
	const QString cacheID = this->createCacheID(x, y, z);
	if (_pendingRequests.contains(cacheID))
		return;
	_pendingRequests.insert(cacheID);

	//Build the request
	const QString fetchURL = url.arg(QString::number(z),
		QString::number(x),
		QString::number(y));
	QNetworkRequest request(QUrl(host + fetchURL));

	//Send the request and setupd a signal to ensure we're notified when it finishes
	QNetworkReply* reply = network->get(request);
	_pendingReplies.insert(reply, cacheID);

	connect(reply,
			SIGNAL(finished()),
			this,
			SLOT(handleNetworkRequestFinished()));
}

void OSMTileSource::handleNetworkRequestFinished()
{
	QObject* sender = QObject::sender();
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender);

	if (reply == 0)
	{
		qWarning() << "QNetworkReply cast failed";
		return;
	}

	reply->deleteLater();
	if (!_pendingReplies.contains(reply))
	{
		qWarning() << "Unknown QNetworkReply";
		return;
	}

	const QString cacheID = _pendingReplies.take(reply);
	_pendingRequests.remove(cacheID);

	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << "Network Error:" << reply->errorString();
		return;
	}

	quint32 x, y, z;
	bool bFlag = TileSource::cacheID2xyz(cacheID, &x, &y, &z);
	if (!bFlag)
	{
		qWarning() << "Failed to convert cacheID " << cacheID << " back to xyz";
		return;
	}

	QByteArray bytes = reply->readAll();
	QImage* image = new QImage();
	if (!image->loadFromData(bytes))
	{
		delete image;
		qWarning() << "failed to make QImage from network byte";
		return;
	}

	QDateTime expireTime;
	if (reply->hasRawHeader("Cache-Control"))
	{
		const QByteArray cacheControl = reply->rawHeader("Cache-Control");
		QRegExp maxAgeFinder("max-age=(\\d+)");
		if (maxAgeFinder.indexIn(cacheControl) != -1)
		{
			bool ok = false;
			const qint64 delta = maxAgeFinder.cap(1).toULongLong(&ok);
			if (ok)
			{
				expireTime = QDateTime::currentDateTime().addSecs(delta);
			}
		}
	}
	this->prepareNewlyReceivedTile(x, y, z, image, expireTime);
}