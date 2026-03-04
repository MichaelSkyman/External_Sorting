#include "data_block.h"
#include <QBrush>

DataBlock::DataBlock(int value, QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setRect(0,0,40,40);
    setBrush(QBrush(Qt::cyan));

    text = new QGraphicsTextItem(QString::number(value), this);
    text->setDefaultTextColor(Qt::black);
    text->setPos(10,5);
}

void DataBlock::moveTo(QPointF pos)
{
    setPos(pos);
}