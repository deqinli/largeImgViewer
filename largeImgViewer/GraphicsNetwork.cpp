#include "GraphicsNetwork.h"
#include <QMutexLocker>
#include <QNetworkRequest>
#include <QThread>
#include <QDebug>

const QByteArray DEFAUL_USER_AGENT = "MapGraphics";

QHash<QThread*, GraphicsNetwork*> GraphicsNetwork::_instances = QHash<QThread*, GraphicsNetwork*>();
QMutex GraphicsNetwork::_mutex;

GraphicsNetwork* GraphicsNetwork::getInstance()
{
	QMutexLocker lock(&_mutex);
	QThread* current = QThread::currentThread();
	if (!GraphicsNetwork::_instances.contains(current))
	{
		GraphicsNetwork::_instances.insert(current, new GraphicsNetwork());
	}

	return GraphicsNetwork::_instances.value(current);
}

GraphicsNetwork::~GraphicsNetwork()
{
	delete _manager;
	_manager = nullptr;
}

QNetworkReply* GraphicsNetwork::get(QNetworkRequest& request)
{
	request.setRawHeader("User-Agent", _userAgent);
	QNetworkReply* toRet = _manager->get(request);
	return toRet;
}

void GraphicsNetwork::setUserAgent(const QByteArray& agent)
{
	_userAgent = agent;
}

QByteArray GraphicsNetwork::userAgent() const
{
	return _userAgent;
}

GraphicsNetwork::GraphicsNetwork()
{
	_manager = new QNetworkAccessManager();
	this->setUserAgent(DEFAUL_USER_AGENT);
}