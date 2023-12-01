#include "TileLayerListModel.h"
#include <QModelIndex>
#include <QtDebug>

TileLayerListModel::TileLayerListModel(QWeakPointer<CompositeTileSource> composite, QObject* parent) : QAbstractListModel(parent), _composite(composite)
{
	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}

	CompositeTileSource* raw = strong.data();

	connect(raw, SIGNAL(sourcesChanged()), this, SLOT(handleCompositeSourcesChanged()));
	connect(raw, SIGNAL(sourceAdded(int)), this, SLOT(handleCompositeSourcesAdded(int)));
	connect(raw, SIGNAL(sourceRemoved(int)), this, SLOT(handleCompositeSourceRemoved(int)));
}

int TileLayerListModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);

	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return 0;
	}
	return strong->numSources();
}

QVariant TileLayerListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
	{
		return QVariant("Invalid index");
	}

	if (index.row()>= this->rowCount())
	{
		return QVariant("Index out if bounds");
	}

	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return QVariant("Null composite");
	}

	if (role == Qt::DisplayRole)
	{
		int i = index.row();
		QSharedPointer<TileSource> tileSource = strong->getSource(i);
		if (tileSource.isNull())
		{
			return QVariant("Error: Null source");
		}
		else
		{
			return tileSource->name();
		}
	}
	else
	{
		return QVariant();
	}
}

Qt::ItemFlags TileLayerListModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return Qt::NoItemFlags;
	}
	return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void TileLayerListModel::handleCompositeSourcesChanged()
{
	QModelIndex topLeft = this->index(0);
	QModelIndex bottomRight = this->index(this->rowCount());
	this->dataChanged(topLeft, bottomRight);
}

void TileLayerListModel::handleCompositeSourcesAdded(int index)
{
	this->beginInsertRows(QModelIndex(), index, index);
	this->endInsertRows();
}

void TileLayerListModel::handleCompositeSourceRemoved(int index)
{
	this->beginRemoveRows(QModelIndex(), index, index);
	this->endRemoveRows();
}