#include "data_block.h"
#include <QPainter>
#include <QFont>
#include <QtMath>

DataBlock::DataBlock(double value, QColor color, qreal height, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_value(value)
    , m_color(color)
    , m_baseColor(color)
    , m_height(height)
    , m_width(28)
    , m_highlighted(false)
    , m_writing(false)
{
    // Format value text
    if(qAbs(value) >= 1000) {
        m_valueStr = QString::number(value, 'e', 0);
    } else if(qAbs(value) >= 100) {
        m_valueStr = QString::number(value, 'f', 0);
    } else {
        m_valueStr = QString::number(value, 'f', 1);
    }
}

QRectF DataBlock::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height + 15);
}

void DataBlock::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw bar
    QColor barColor = m_color;
    if(m_highlighted) {
        barColor = QColor(255, 255, 0);  // Yellow highlight
    }
    if(m_writing) {
        barColor = barColor.lighter(150);
    }
    
    QRectF barRect(0, 0, m_width, m_height);
    painter->setBrush(barColor);
    painter->setPen(QPen(barColor.darker(130), 1));
    painter->drawRoundedRect(barRect, 3, 3);
    
    // Draw highlight glow
    if(m_highlighted) {
        painter->setPen(QPen(QColor(255, 255, 0, 100), 3));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(barRect.adjusted(-2, -2, 2, 2), 4, 4);
    }
    
    // Draw value text below bar
    painter->setPen(Qt::white);
    QFont font("Segoe UI", 7);
    painter->setFont(font);
    QRectF textRect(0, m_height + 2, m_width, 12);
    painter->drawText(textRect, Qt::AlignCenter, m_valueStr);
}

void DataBlock::setColor(const QColor& color)
{
    m_color = color;
    m_baseColor = color;
    update();
}

void DataBlock::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    update();
}

void DataBlock::setWriteAnimation(bool writing)
{
    m_writing = writing;
    update();
}

void DataBlock::setBarHeight(qreal h)
{
    prepareGeometryChange();
    m_height = h;
    update();
}
