#pragma once
#include <QStyledItemDelegate>
#include <QWeakPointer>
#include "CompositeTileSource.h"

class TileSourceDelegate : public QStyledItemDelegate
{
	Q_OBJECT;
public:
	explicit TileSourceDelegate(QWeakPointer<CompositeTileSource> composite, QObject* parent = 0);

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	QWeakPointer<CompositeTileSource> _composite;
};

