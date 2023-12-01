#pragma once
#include <QAbstractListModel>
#include <QWeakPointer>
#include "CompositeTileSource.h"

class TileLayerListModel : public QAbstractListModel
{
	Q_OBJECT;
public:
	explicit TileLayerListModel(QWeakPointer<CompositeTileSource> composite, QObject* parent = 0);

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;

private slots:
	void handleCompositeSourcesChanged();
	void handleCompositeSourcesAdded(int index);
	void handleCompositeSourceRemoved(int index);

private:
	QWeakPointer<CompositeTileSource> _composite;

};

