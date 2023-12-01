#pragma once
#include <QMutex>
#include <QNetworkAccessManager>
#include <QHash>

class GraphicsNetwork
{
public:
	static GraphicsNetwork* getInstance();
	~GraphicsNetwork();

	QNetworkReply* get(QNetworkRequest& request);
	void setUserAgent(const QByteArray& agent);
	QByteArray userAgent() const;

protected:
	GraphicsNetwork();

private:
	static QHash<QThread*, GraphicsNetwork*> _instances;
	static QMutex _mutex;
	QNetworkAccessManager* _manager;
	QByteArray _userAgent;
};

