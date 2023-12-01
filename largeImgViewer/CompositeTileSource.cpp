#include "CompositeTileSource.h"
#include <QDebug>
#include <QPainter>
#include <QMutexLocker>
#include <QThread>
#include <QPointer>
#include <Qtimer>

CompositeTileSource::CompositeTileSource() : TileSource()
{
	_globalMutex = new QMutex(QMutex::Recursive);
	this->setCacheMode(TileSource::NoCaching);
}

CompositeTileSource::~CompositeTileSource()
{
	_globalMutex->lock();
	qDebug() << this << "destructing";
	this->clearPendingTiles();

	QList<QPointer<QThread>> tileSourceThreads;
	for each (QSharedPointer<TileSource> source in _childSources)
	{
		tileSourceThreads.append(QPointer<QThread>(source->thread()));
	}

	_childSources.clear();

	int numThreads = tileSourceThreads.size();
	for (int i = 0; i < numThreads; i++)
	{
		QPointer<QThread> thread = tileSourceThreads[i];
		if (!thread.isNull() && thread != this->thread())
		{
			thread->wait(10000);
		}
	}
	delete this->_globalMutex;
}

QPointF CompositeTileSource::ll2qgs(const QPointF& ll, quint8 zoomLevel) const
{
	QMutexLocker lock(_globalMutex);
	if (_childSources.isEmpty())
	{
		qWarning() << "Composite tile source is empty -- result undefined";
		return QPointF(0, 0);
	}

	return _childSources.at(0)->ll2qgs(ll, zoomLevel);
}

QPointF CompositeTileSource::qgs2ll(const QPointF& qgs, quint8 zoomLevel) const
{
	QMutexLocker lock(_globalMutex);
	if (_childSources.isEmpty())
	{
		qWarning() << "Composite tile source is empty -- result undefined";
		return QPointF(0, 0);
	}
	return _childSources.at(0)->qgs2ll(qgs, zoomLevel);
}

quint64 CompositeTileSource::tilesNumOnZoomLevel(quint8 zoomLevel) const
{
	QMutexLocker lock(_globalMutex);
	if (_childSources.isEmpty())
	{
		return 1;
	}
	else
	{
		return _childSources.at(0)->tilesNumOnZoomLevel(zoomLevel);
	}
}

quint16 CompositeTileSource::getTileSize() const
{
	QMutexLocker lock(_globalMutex);
	if (_childSources.isEmpty())
	{
		return 256;
	}
	else
	{
		return _childSources.at(0)->getTileSize();
	}
}

quint8 CompositeTileSource::minZoomLevel(QPointF ll)
{
	QMutexLocker lock(_globalMutex);
	quint8 highest = 0;

	for each (QSharedPointer<TileSource> source in _childSources)
	{
		quint8 current = source->minZoomLevel(ll);
		if (current > highest)
		{
			highest = current;
		}
	}
	return highest;
}

quint8 CompositeTileSource::maxZoomLevel(QPointF ll)
{
	QMutexLocker lock(_globalMutex);
	quint8 lowest = 50;
	for each (QSharedPointer<TileSource> source in _childSources)
	{
		quint8 current = source->maxZoomLevel(ll);
		if (current < lowest)
		{
			lowest = current;
		}
	}
	return lowest;
}

QString CompositeTileSource::name() const
{
	return "Composite Tile Source";
}

QString CompositeTileSource::tileFileExtension() const
{
	return ".jpg";
}

void CompositeTileSource::addSourceTop(QSharedPointer<TileSource> source, qreal opacity)
{
	QMutexLocker lock(_globalMutex);
	if (source.isNull())
	{
		return;
	}

	this->doChildThreading(source);

	_childSources.insert(0, source);
	_childOpacities.insert(0, opacity);
	_childEnabledFlags.insert(0, true);

	connect(source.data(), 
		SIGNAL(tileRetrieved(quint32, quint32, quint8)),
		this,
		SLOT(handleTileRetriecved(quint32,quint32,quint8)));

	this->sourceAdded(0);
	this->sourcesChanged();
	this->allTilesInvalidated();
}

void CompositeTileSource::addSourceBottom(QSharedPointer<TileSource> source, qreal opaciy)
{
	QMutexLocker lock(_globalMutex);
	if (source.isNull())
	{
		return;
	}
	this->doChildThreading(source);

	_childSources.append(source);
	_childOpacities.append(opaciy);
	_childEnabledFlags.append(true);

	connect(source.data(),
		SIGNAL(tileRetrieved(quint32, quint32, quint8)),
		this,
		SLOT(handleTileRetriecved(quint32, quint32, quint8)));

	this->sourceAdded(_childSources.size() - 1);
	this->sourcesChanged();
	this->allTilesInvalidated();
}

void CompositeTileSource::moveSource(int from, int to)
{
	if (from < 0 || to < 0)
	{
		return;
	}
	QMutexLocker lock(_globalMutex);

	int size = this->numSources();
	if (from >= size || to >= size)
	{
		return;
	}

	_childSources.move(from, to);
	_childOpacities.move(from, to);
	_childEnabledFlags.move(from, to);

	this->sourceReordered();
	this->sourcesChanged();
	this->allTilesInvalidated();
}

void CompositeTileSource::removeSource(int index)
{
	QMutexLocker lock(_globalMutex);
	if (index < 0 || index >= _childSources.size())
	{
		return;
	}
	_childSources.removeAt(index);
	_childOpacities.removeAt(index);
	_childEnabledFlags.removeAt(index);
	
	this->clearPendingTiles();
	
	this->sourceRemoved(index);
	this->sourcesChanged();
	this->allTilesInvalidated();
}

int CompositeTileSource::numSources() const
{
	QMutexLocker lock(_globalMutex);
	return _childSources.size();
}

QSharedPointer<TileSource> CompositeTileSource::getSource(int index) const
{
	QMutexLocker lock(_globalMutex);
	if (index < 0 || index >= _childSources.size())
	{
		return QSharedPointer<TileSource>();
	}

	return _childSources[index];
}

qreal CompositeTileSource::getOpacity(int index) const
{
	QMutexLocker lock(_globalMutex);
	if (index < 0 || index >= _childSources.size())
	{
		return 0.0;
	}
	return _childOpacities[index];
}

void CompositeTileSource::setOpacity(int index, qreal opacity)
{
	opacity = qMin<qreal>(1.0, qMax<qreal>(0.0, opacity));
	QMutexLocker lock(_globalMutex);
	if (index < 0 || index >= _childSources.size())
	{
		return;
	}
	if (_childOpacities[index] == opacity)
	{
		return;
	}

	_childOpacities[index = opacity];

	this->sourcesChanged();
	this->allTilesInvalidated();
}

bool CompositeTileSource::getEnabledFlag(int index) const
{
	QMutexLocker lock(_globalMutex);
	if (index<0||index>=_childSources.size())
	{
		return 0;
	}
	return _childEnabledFlags[index];
}

void CompositeTileSource::setEnabledFlag(int index, bool isEnabled)
{
	QMutexLocker lock(_globalMutex);
	if (index < 0 || index >= _childSources.size())
	{
		return;
	}
	if (_childEnabledFlags[index] == isEnabled)
	{
		return;
	}
	_childEnabledFlags[index] = isEnabled;
	this->sourcesChanged();
	this->allTilesInvalidated();
}

void CompositeTileSource::fetchTile(quint32 x, quint32 y, quint8 z)
{
	QMutexLocker lock(_globalMutex);
	if (_childSources.isEmpty())
	{
		QImage* toDispImg = new QImage(	this->getTileSize(),
										this->getTileSize(),
										QImage::Format_ARGB32_Premultiplied);
		QPainter painter(toDispImg);
		painter.fillRect(toDispImg->rect(), Qt::white);
		painter.drawText(toDispImg->rect(), QString("Composite Source Empty"), QTextOption(Qt::AlignCenter));
		painter.end();
		this->prepareNewlyReceivedTile(x, y, z, toDispImg);
		return;
	}
	QString cacheID = this->createCacheID(x, y, z);
	if (_pendingTiles.contains(cacheID))
	{
		QMap<quint32, QImage*>* tiles = _pendingTiles.value(cacheID);
		for each (QImage*tile in *tiles)
		{
			delete tile;
		}
		tiles->clear();
	}
	else
	{
		_pendingTiles.insert(cacheID, new QMap<quint32, QImage*>());
	}
	for (int i = 0; i < _childSources.size(); i++)
	{
		QSharedPointer<TileSource> child = _childSources.at(i);
		child->requestTile(x, y, z);
	}
}

void CompositeTileSource::handleTileRetriecved(quint32 x, quint32 y, quint8 z)
{
	QMutexLocker lock(_globalMutex);
	QObject* sender = QObject::sender();

	TileSource* tileSource = qobject_cast<TileSource*>(sender);

	if (!tileSource)
	{
		qWarning() << this << "failed TileSource cast";
		return;
	}
	
	int tileSourceIndex = -1;
	for (int i = 0; i < _childSources.size(); i++)
	{
		if (_childSources[i].data() != tileSource)
			continue;
		tileSourceIndex = i;
		break;
	}
	if (tileSourceIndex == 1)
	{
		qWarning() << this << "recieved tile from unknown source...";
		return;
	}

	const QString cacheID = this->createCacheID(x, y, z);
	if (!_pendingTiles.contains(cacheID))
	{
		qWarning() << this << "retrieved unknown tile" << x << y << z << "from" << tileSource;
		return;
	}
	QImage* tile = tileSource->getFinishedTile(x, y, z);
	if (!tile)
	{
		qWarning() << this << "received null tile" << x << y << z << "from" << tileSource;
		return;
	}

	QMap<quint32, QImage*>* tiles = _pendingTiles.value(cacheID);
	if (tiles->contains(tileSourceIndex))
	{
		delete tile;
		return;
	}
	tiles->insert(tileSourceIndex, tile);

	if (tiles->size() < _childSources.size())
	{
		return;
	}
	QImage* toDispImg = new QImage( this->getTileSize(),
									this->getTileSize(),
									QImage::Format_ARGB32_Premultiplied);

	QPainter painter(toDispImg);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.setOpacity(1.0);
	for (int i = tiles->size() - 1; i >= 0; i--)
	{
		QImage* childTile = tiles->value(i);
		qreal opacity = _childOpacities[i];

		if (this->numSources() == 1)
		{
			opacity = 1.0;
		}
		if (_childEnabledFlags[i] == false)
		{
			opacity = 0.0;
		}
		painter.setOpacity(opacity);
		painter.drawImage(0, 0, *childTile);
		delete childTile;
	}
	delete tiles;
	_pendingTiles.remove(cacheID);
	painter.end();

	this->prepareNewlyReceivedTile(x, y, z, toDispImg);
}

void CompositeTileSource::clearPendingTiles()
{
	QList<QMap<quint32, QImage*>*> pendingTiles = _pendingTiles.values();
	for (int i = 0; i < pendingTiles.size(); i++)
	{
		QMap<quint32, QImage*>* tiles = pendingTiles.at(i);
		for each (QImage* tile in tiles->values())
		{
			delete tile;
		}
		delete tiles;
	}
	_pendingTiles.clear();
}

void CompositeTileSource::doChildThreading(QSharedPointer<TileSource> source)
{
	if (source.isNull())
	{
		return;
	}

	QThread* sourceThread = new QThread();
	sourceThread->start();
	source->moveToThread(sourceThread);

	connect(source.data(),
		    SIGNAL(destroyed()),
		    sourceThread,
		    SLOT(quit()));
	connect(sourceThread,
			SIGNAL(finished()),
			sourceThread,
			SLOT(deleteLater()));
}