#include "TileSourceDelegate.h"
#include <QPainter>
#include <QDebug>
#include <QPushButton>
#include <QStringBuilder>

TileSourceDelegate::TileSourceDelegate(QWeakPointer<CompositeTileSource> composite, QObject* paren) :QStyledItemDelegate(paren), _composite(composite)
{

}

void TileSourceDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}

	QSharedPointer<TileSource> childSource = strong->getSource(index.row());
	if (childSource.isNull())
	{
		return;
	}

	painter->save();

	QPalette palette = option.palette;

	QRect rect = option.rect;
	rect.setWidth(rect.width() - 2);

	QBrush backgroundBrush = palette.base();
	QColor BorderColor = palette.text().color();
	QColor textColor = palette.text().color();
	if (option.state & QStyle::State_Selected)
	{
		backgroundBrush = palette.highlight();
	}
	painter->fillRect(rect, backgroundBrush);
	painter->setPen(BorderColor);
	painter->drawRect(rect);

	QFont nameFont = painter->font();
	QFont otherFont = painter->font();
	nameFont.setPointSize(nameFont.pointSize() + 2);
	nameFont.setBold(true);

	QRect TextRect = rect;
	TextRect.adjust(1, 0, -1, 0);
	QString nameString = childSource->name();
	painter->setPen(textColor);
	painter->setFont(nameFont);
	painter->drawText(TextRect, nameString, QTextOption(Qt::AlignLeft | Qt::AlignTop));

	QString opacityString = "Opacity:" % QString::number(strong->getOpacity(index.row()), 'f', 2);
	painter->setPen(textColor);
	painter->setFont(otherFont);
	painter->drawText(TextRect, opacityString, QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

	QString state = "Enabled";
	if (strong->getEnabledFlag(index.row()) == false)
	{
		state = "Disabled";
	}
	QString stateString = "Status:" % state;
	painter->setPen(textColor);
	painter->setFont(otherFont);
	painter->drawText(TextRect, stateString, QTextOption(Qt::AlignLeft | Qt::AlignBottom));
	painter->restore();
}

QSize TileSourceDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);

	QSize toRet;
	toRet.setWidth(150);
	toRet.setHeight(50);
	return toRet;
}