#pragma once

#include <QSharedPointer>

#include "TileSource.h"

class PrivateQGraphicsInfoSource
{
public:
    virtual quint8 zoomLevel() const = 0;

    virtual QSharedPointer<TileSource> tileSource() const = 0;
};

