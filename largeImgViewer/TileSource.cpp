#include "TileSource.h"
#include <QStringBuilder>
#include <QMutexLocker>
#include <QtDebug>
#include <QStringList>
#include <QDataStream>

const QString TILES_CACHE_FOLDER_DIR = ".TilesCache";
const QString TILES_DIR = QDir::currentPath() + "/TileMap/";
const quint32 DEFAULT_CACHE_DAYS = 3650;
const quint64 MAX_DISK_CACHE_READ_ATTEMPTS = 100000;


TileSource::TileSource() :QObject()
{
	this->_cacheExpirationsLoaded = false;
	this->setCacheMode(DiskAndMemCaching);
	connect(this,
		SIGNAL(tileRequested(quint32, quint32, quint8)),
		this,
		SLOT(startTileRequest(quint32, quint32, quint8)),
		Qt::QueuedConnection);

	connect(this,
		SIGNAL(allTilesInvalidated()),
		this,
		SLOT(clearTempCache()));

}

TileSource::~TileSource()
{
	this->saveCacheExpirationToDisk();
}

void TileSource::requestTile(quint32 x, quint32 y, quint8 z)
{
	this->tileRequested(x, y, z);
}

QImage* TileSource::getFinishedTile(quint32 x, quint32 y, quint8 z)
{
	const QString cacheID = this->createCacheID(x, y, z);
	QMutexLocker lock(&_tempCacheLock);
	if (!_tempCache.contains(cacheID))
	{
		qWarning() << __FUNCTION__ << " : " << __LINE__ << "ERROR : there is no tiles to get!";
		return 0;
	}
	return _tempCache.take(cacheID);
}

TileSource::CacheMode TileSource::getCacheMode() const
{
	return _cacheMode;
}

void TileSource::setCacheMode(TileSource::CacheMode nMode)
{
	_cacheMode = nMode;
}

void TileSource::startTileRequest(quint32 x, quint32 y, quint8 z)
{
	if (this->getCacheMode() == DiskAndMemCaching)
	{
		const QString cacheID = TileSource::createCacheID(x, y, z);
		QImage* cachedImg = this->fromMemCache(cacheID);
		if (!cachedImg)
		{
			cachedImg = this->fromDiskCache(cacheID);
		}

		if (cachedImg)
		{
			this->prepareRetrievedTile(x, y, z, cachedImg);
			return;
		}
	}

	this->fetchTile(x, y, z);
}

void TileSource::clearTempCache()
{
	_tempCache.clear();
}

QString TileSource::createCacheID(quint32 x, quint32 y, quint8 z)
{
	QString qstrTileID = QString::number(x) % "," % QString::number(y) % "," % QString::number(z);
	return qstrTileID;
}

bool TileSource::cacheID2xyz(const QString& qstring, quint32* x, quint32* y, quint32* z)
{
	QStringList list = qstring.split(',');
	if (list.size() != 3)
	{
		qWarning() << "Bad cacheID " << qstring << " cannot convert";
		return false;
	}

	bool ok = true;
	*x = list.at(0).toUInt(&ok);
	if (!ok)
	{
		return false;
	}
	*y = list.at(1).toUInt(&ok);
	if (!ok)
	{
		return false;
	}
	*z = list.at(2).toUInt(&ok);
	return ok;
}

QImage* TileSource::fromMemCache(const QString& cacheID)
{
	QImage* toDispImg = 0;
	if (_memoryCahce.contains(cacheID))
	{
		QDateTime expireTime = this->getTileExpirationTime(cacheID);

		if (QDateTime::currentDateTime().secsTo(expireTime) <= 0)
		{
			_memoryCahce.remove(cacheID);
		}
		else
		{
			toDispImg = new QImage(*_memoryCahce.object(cacheID));
		}
	}
	return toDispImg;
}

void TileSource::toMemCache(const QString& cacheID, QImage* toCache, const QDateTime& expireTime)
{
	if (toCache == 0)
		return;
	if (_memoryCahce.contains(cacheID))
		return;

	this->setTileExpirationTime(cacheID, expireTime);
	QImage* imgCopy = new QImage(*toCache);
	_memoryCahce.insert(cacheID, imgCopy);
}

QImage* TileSource::fromDiskCache(const QString& cacheID)
{
	quint32 x, y, z;
	if (!TileSource::cacheID2xyz(cacheID, &x, &y, &z))
	{
		return 0;
	}

	const QString path = this->getDistCacheFile(x, y, z);
	QFile fp(path);
	if (!fp.exists())
	{
		return 0;
	}

	QDateTime expireTime = this->getTileExpirationTime(cacheID);
	if (QDateTime::currentDateTime().secsTo(expireTime) <= 0)
	{
		if (!QFile::remove(path))
		{
			qWarning() << "Failed to remove old cache file" << path;
			return 0;
		}
	}

	if (!fp.open(QFile::ReadOnly))
	{
		qWarning() << "Failed to open" << QFileInfo(fp.fileName()).baseName() << "from cache";
		return 0;
	}

	QByteArray data;
	quint64 counter = 0;
	while (data.length() < fp.size())
	{
		data += fp.read(20480);
		if (++counter >= MAX_DISK_CACHE_READ_ATTEMPTS)
		{
			qWarning() << "Reading cache file" << fp.fileName() << ", file is too large to load. Aborting.";
			return 0;
		}
	}

	QImage* image = new QImage();
	if (!image->loadFromData(data))
	{
		delete image;
		return 0;
	}

	return image;
}

void TileSource::toDiskCache(const QString& cacheID, QImage* toCacheImg, const QDateTime& expireTime)
{
	quint32 x, y, z;
	if (!TileSource::cacheID2xyz(cacheID, &x, &y, &z))
		return;

	const QString filePath = this->getDistCacheFile(x, y, z);

	QFile fp(filePath);
	if (fp.exists())
		return;
	this->setTileExpirationTime(cacheID, expireTime);

	const char* format = 0;

	const int quality = 100;

	if (!toCacheImg->save(filePath, format, quality))
		qWarning() << "Failed to put" << this->name() << x << y << z << "into disk cache";
}

void TileSource::prepareRetrievedTile(quint32 x, quint32 y, quint8 z, QImage* image)
{
	if (image == 0)
		return;

	QMutexLocker lock(&_tempCacheLock);

	_tempCache.insert(TileSource::createCacheID(x, y, z), image);

	lock.unlock();

	this->tileRetrieved(x, y, z);
}

void TileSource::prepareNewlyReceivedTile(quint32 x, quint32 y, quint8 z, QImage* image, QDateTime expireTime)
{
	const QString cacheID = TileSource::createCacheID(x, y, z);
	if (this->getCacheMode() == DiskAndMemCaching)
	{
		this->toMemCache(cacheID, image, expireTime);
		this->toDiskCache(cacheID, image, expireTime);
	}
	this->prepareRetrievedTile(x, y, z, image);
}

QDateTime TileSource::getTileExpirationTime(const QString& cacheID)
{
	this->loadCacheExpirationFromDisk();

	QDateTime expireTime;
	if (_cacheExpirations.contains(cacheID))
	{
		expireTime = _cacheExpirations.value(cacheID);
	}
	else
	{
		qWarning() << "Tile" << cacheID << "has unknown expire time. Resetting to default of " << DEFAULT_CACHE_DAYS << " days";
		expireTime = QDateTime::currentDateTime().addDays(DEFAULT_CACHE_DAYS);
		_cacheExpirations.insert(cacheID, expireTime);
	}
	return expireTime;
}

void TileSource::setTileExpirationTime(const QString& cacheID, QDateTime expireTime)
{
	this->loadCacheExpirationFromDisk();

	if (expireTime.isNull())
	{
		expireTime = QDateTime::currentDateTime().addDays(DEFAULT_CACHE_DAYS);
	}
	_cacheExpirations.insert(cacheID, expireTime);
}

QDir TileSource::getDiskCacheDirectory(quint32 x, quint32 y, quint8 z) const
{
	Q_UNUSED(x);
	QString pathString = TILES_DIR + QString("LEVEL_%1/R%2").arg(z + 1).arg(y, 6, 10, QLatin1Char('0'));
	QDir toDispImgDir = QDir(pathString);
	if (!toDispImgDir.exists())
	{
		if (!toDispImgDir.mkpath(toDispImgDir.absolutePath()))
		{
			qWarning() << "Failed to create cache directory" << toDispImgDir.absolutePath();
		}
	}
	return toDispImgDir;
}

QString TileSource::getDistCacheFile(quint32 x, quint32 y, quint8 z)const
{
	QString qstrTilePath = this->getDiskCacheDirectory(x, y, z).absolutePath() % "/" % QString("C%1").arg(x, 6, 10, QLatin1Char('0')) + ".JPG";
	return qstrTilePath;
}

void TileSource::loadCacheExpirationFromDisk()
{
	if (_cacheExpirationsLoaded)
	{
		return;
	}

	_cacheExpirationsLoaded = true;
	QDir dir = this->getDiskCacheDirectory(0, 0, 0);
	QString path = dir.absolutePath() % "/" % "cacheExpirations.db";
	_cacheExpirationsFile = path;

	QFile fp(path);
	if (!fp.exists())
	{
		return;
	}
	if (!fp.open(QIODevice::ReadOnly))
	{
		qWarning() << "Failed to open cache expiration file for reading:" << fp.errorString();
		return;
	}

	QDataStream stream(&fp);
	stream >> _cacheExpirations;
}

void TileSource::saveCacheExpirationToDisk()
{
#if 1
	if (!_cacheExpirationsLoaded || _cacheExpirationsFile.isEmpty())
	{
		return;
	}
	QFile fp(_cacheExpirationsFile);
	if (!fp.open(QIODevice::Truncate | QIODevice::WriteOnly))
	{
		qWarning() << "Failed to open expiration file for writing:" << fp.errorString();
		return;
	}

	QDataStream stream(&fp);
	stream << _cacheExpirations;
	qDebug() << "Cache expirations saved to" << _cacheExpirationsFile;
#endif
}