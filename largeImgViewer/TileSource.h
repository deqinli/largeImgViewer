#pragma once
#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QImage>
#include <QCache>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QMap>


class TileSource : public QObject
{
	Q_OBJECT;
public:
	enum CacheMode
	{
		NoCaching,
		DiskAndMemCaching
	};

public:
	explicit TileSource();
	virtual ~TileSource();

	void requestTile(quint32 x, quint32 y, quint8 z);
	QImage* getFinishedTile(quint32 x, quint32 y, quint8 z);

	TileSource::CacheMode getCacheMode() const;

	void setCacheMode(TileSource::CacheMode);

	virtual QPointF ll2qgs(const QPointF& ll, quint8 zoomLevel) const = 0;

	virtual QPointF qgs2ll(const QPointF& qgs, quint8 zoomLevel) const = 0;

	virtual quint64 tilesNumOnZoomLevel(quint8 zomLevel) const = 0;

	virtual quint16 getTileSize() const = 0;

	virtual quint8 minZoomLevel(QPointF ll = QPointF()) = 0;

	virtual quint8 maxZoomLevel(QPointF ll = QPoint()) = 0;

	virtual QString name() const = 0;

	virtual QString tileFileExtension() const = 0;

private:
	void prepareRetrievedTile(quint32 x, quint32 y, quint8 z, QImage* image);
	
	QDir getDiskCacheDirectory(quint32 x, quint32 y, quint8 z) const;

	QString getDistCacheFile(quint32 x, quint32 y, quint8 z) const;

	void loadCacheExpirationFromDisk();
	
	void saveCacheExpirationToDisk();



signals:
	void tileRetrieved(quint32 x, quint32 y, quint8 z);

	void tileRequested(quint32 x, quint32 y, quint8 z);

	void allTilesInvalidated();


private slots:
	void startTileRequest(quint32 x, quint32 y, quint8 z);
	
	void clearTempCache();


protected:
	static QString createCacheID(quint32 x, quint32 y, quint8 z);

	static bool cacheID2xyz(const QString& strID, quint32* x, quint32* y, quint32* z);

	QImage* fromMemCache(const QString& cacheID);

	void toMemCache(const QString& cacheID, QImage* toCache, const QDateTime& expireTime = QDateTime());

	QImage* fromDiskCache(const QString& cacheID);

	void toDiskCache(const QString& cacheID, QImage* toCacheImg, const QDateTime& expireTime = QDateTime());

	virtual void fetchTile(quint32 x, quint32 y, quint8 z) = 0;

	void prepareNewlyReceivedTile(quint32 x, quint32 y, quint8 z, QImage* image, QDateTime expireTime = QDateTime());

	QDateTime getTileExpirationTime(const QString& cacheID);

	void setTileExpirationTime(const QString& cacheID, QDateTime expireTime);



private:
	bool _cacheExpirationsLoaded;

	QString _cacheExpirationsFile;

	TileSource::CacheMode _cacheMode;

	QCache<QString, QImage> _tempCache;

	QMutex _tempCacheLock;

	QCache<QString, QImage> _memoryCahce;

	QHash<QString, QDateTime> _cacheExpirations;
};

