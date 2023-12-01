#include "CompositeTileSourceWidget.h"
#include "TileLayerListModel.h"
#include "TileSourceDelegate.h"
#include "OSMTileSource.h"
#include <QDebug>
#include <QMenu.h>

CompositeTileSourceWidget::CompositeTileSourceWidget(QWeakPointer<CompositeTileSource> composite, QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::CompositeTileSourceWidgetClass())
{
	this->_composite = composite;
	ui->setupUi(this);
	this->init();
}

CompositeTileSourceWidget::~CompositeTileSourceWidget()
{
	delete ui;
}

void CompositeTileSourceWidget::setComposite(QWeakPointer<CompositeTileSource> nComposite)
{
	_composite = nComposite;
	this->init();
}

void CompositeTileSourceWidget::handleCurrentSelectionChanged(QModelIndex current, QModelIndex previous)
{
	Q_UNUSED(previous);
	bool enableGUI = current.isValid();

	this->ui->removeSourceButton->setEnabled(enableGUI);
	this->ui->opacitySlider->setEnabled(enableGUI);

	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}

	this->ui->moveDownButton->setEnabled(enableGUI && (strong->numSources() - 1) > current.row());
	this->ui->moveUpButton->setEnabled(enableGUI && 0 < current.row());

	if (enableGUI)
	{
		qreal opacityFloat = strong->getOpacity(current.row());
		this->ui->opacitySlider->setValue(opacityFloat * 100);
	}
}

void CompositeTileSourceWidget::handleCompositeChange()
{
	QItemSelectionModel* selModel = this->ui->listView->selectionModel();
	QModelIndex index = selModel->currentIndex();

	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}
	qreal opacityFloat = strong->getOpacity(index.row());
	this->ui->opacitySlider->setValue(opacityFloat * 100);
}

void CompositeTileSourceWidget::addOSMTileLayer()
{
	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}
	QSharedPointer<OSMTileSource> source(new OSMTileSource(OSMTileSource::OSMTiles));
	strong->addSourceTop(source);
}

void CompositeTileSourceWidget::on_removeSourceButton_clicked()
{
	QItemSelectionModel* selModel = this->ui->listView->selectionModel();
	QModelIndex index = selModel->currentIndex();

	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}

	strong->removeSource(index.row());
	selModel->clear();
}

void CompositeTileSourceWidget::on_opacitySlider_valueChanged(int value)
{
	QItemSelectionModel* selModel = this->ui->listView->selectionModel();
	QModelIndex index = selModel->currentIndex();
	if (!index.isValid())
	{
		return;
	}

	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}

	qreal opacityFloat = (qreal)value / 100.0;
	strong->setOpacity(index.row(), opacityFloat);
}

void CompositeTileSourceWidget::on_moveDownButton_clicked()
{
	QItemSelectionModel* selModel = this->ui->listView->selectionModel();
	QModelIndex index = selModel->currentIndex();
	if (!index.isValid())
	{
		return;
	}

	QSharedPointer<CompositeTileSource> strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}

	int numberOfLayers = strong->numSources();
	int currentIndex = index.row();
	int desiredIndex = qMin<int>(numberOfLayers - 1, currentIndex + 1);
	strong->moveSource(currentIndex, desiredIndex);
	selModel->setCurrentIndex(selModel->model()->index(desiredIndex, 0),QItemSelectionModel::SelectCurrent);
}

void CompositeTileSourceWidget::on_moveUpButton_cicked()
{
	QItemSelectionModel* selModel = this->ui->listView->selectionModel();
	QModelIndex index = selModel->currentIndex();

	if (!index.isValid())
	{
		return;
	}

	QSharedPointer<CompositeTileSource>	 strong = _composite.toStrongRef();
	if (strong.isNull())
	{
		return;
	}

	int currentIndex = index.row();
	int desiredIndex = qMin<int>(0, currentIndex - 1);
	strong->moveSource(currentIndex, desiredIndex);
	selModel->setCurrentIndex(selModel->model()->index(desiredIndex, 0), QItemSelectionModel::SelectCurrent);
}

void CompositeTileSourceWidget::init()
{
	TileSourceDelegate* delegate = new TileSourceDelegate(_composite, this);
	TileLayerListModel* model = new TileLayerListModel(_composite, this);
	QAbstractItemModel* oldModel = this->ui->listView->model();
	QAbstractItemDelegate* oldDelegate = this->ui->listView->itemDelegate();

	this->ui->listView->setModel(model);
	this->ui->listView->setItemDelegate(delegate);

	if (oldModel != nullptr)
	{
		delete oldModel;
	}
	if (oldDelegate != nullptr)
	{
		delete oldDelegate;
	}

	QItemSelectionModel* selModel = this->ui->listView->selectionModel();
	connect(selModel, SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(handleCurrentSelectionChanged(QModelIndex, QModelIndex)));
	QMenu* menu = new QMenu(this->ui->addSourceButton);
	menu->addAction("openStreetMap Tiles", this, SLOT(addOSMTileLayer()));
	this->ui->addSourceButton->setMenu(menu);
}

