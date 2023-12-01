#pragma once
#include <QWidget>
#include "ui_CompositeTileSourceWidget.h"
#include <QWeakPointer>
#include "CompositeTileSource.h"


QT_BEGIN_NAMESPACE
namespace Ui { class CompositeTileSourceWidgetClass; };
QT_END_NAMESPACE

class CompositeTileSourceWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CompositeTileSourceWidget(QWeakPointer<CompositeTileSource> composite = QWeakPointer<CompositeTileSource>(), QWidget *parent = nullptr);
	~CompositeTileSourceWidget();

	void setComposite(QWeakPointer<CompositeTileSource> nComposite);

private slots:
	void handleCurrentSelectionChanged(QModelIndex, QModelIndex);
	void handleCompositeChange();
	void addOSMTileLayer();
	void on_removeSourceButton_clicked();
	void on_opacitySlider_valueChanged(int value);
	void on_moveDownButton_clicked();
	void on_moveUpButton_cicked();

private:
	void init();
	Ui::CompositeTileSourceWidgetClass *ui;
	QWeakPointer<CompositeTileSource> _composite;
};
