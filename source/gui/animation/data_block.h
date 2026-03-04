#pragma once
#include <QGraphicsObject>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QColor>

class DataBlock : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(qreal barHeight READ barHeight WRITE setBarHeight)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    DataBlock(double value, QColor color = Qt::cyan, qreal height = 40.0, QGraphicsItem* parent = nullptr);
    
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
    void setColor(const QColor& color);
    void setHighlighted(bool highlighted);
    void setWriteAnimation(bool writing);
    
    qreal barHeight() const { return m_height; }
    void setBarHeight(qreal h);
    
    double value() const { return m_value; }

private:
    double m_value;
    QColor m_color;
    QColor m_baseColor;
    qreal m_height;
    qreal m_width;
    bool m_highlighted;
    bool m_writing;
    QString m_valueStr;
};
