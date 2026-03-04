#pragma once
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

/**
 * @brief A labeled rectangular graphics item representing a single data value.
 *
 * Renders as a colored rectangle with a numeric text label, used in the
 * legacy QGraphicsScene-based animation view.
 */
class DataBlock : public QGraphicsRectItem
{
public:
    /// @brief Constructs a DataBlock displaying @p value.
    /// @param value  Integer value rendered as the block's text label.
    /// @param parent Optional parent graphics item.
    DataBlock(int value, QGraphicsItem* parent = nullptr);

    /// @brief Immediately moves the block to the scene position @p pos.
    /// @param pos Target scene position.
    void moveTo(QPointF pos);

private:
    QGraphicsTextItem* text; ///< Text label showing the block's numeric value.
};