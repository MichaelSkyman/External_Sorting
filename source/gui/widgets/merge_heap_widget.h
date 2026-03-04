#pragma once

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QVector>
#include <QPointF>
#include <QTimer>

// Heap node for visualization
struct HeapNode {
    double value = 0.0;
    int runIndex = -1;          // Which run this value came from
    QPointF position;           // Visual position
    QPointF targetPosition;     // Animation target
    qreal scale = 1.0;
    qreal opacity = 1.0;
    bool highlighted = false;
    bool isExtracting = false;
    bool isSwapping = false;
    QColor color;
    
    HeapNode() = default;
    HeapNode(double val, int run) : value(val), runIndex(run) {}
};

// Animates and renders the min-heap used during merge phase
class MergeHeapWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal animProgress READ animProgress WRITE setAnimProgress)

public:
    explicit MergeHeapWidget(QWidget* parent = nullptr);
    
    // Heap operations
    void setHeap(const QVector<HeapNode>& nodes);
    void clear();
    
    // Animated operations
    void animateExtractMin();
    void animateInsert(double value, int runIndex);
    void animateSwap(int idx1, int idx2);
    void animateHeapify(int index);
    void highlightMin();
    void clearHighlights();
    
    // Visual configuration
    void setNodeRadius(qreal radius) { m_nodeRadius = radius; update(); }
    void setLevelSpacing(qreal spacing) { m_levelSpacing = spacing; recalculatePositions(); }
    void setNodeSpacing(qreal spacing) { m_nodeSpacing = spacing; recalculatePositions(); }
    
    // State
    int heapSize() const { return m_nodes.size(); }
    bool isAnimating() const { return m_animating; }
    
    // Animation progress property
    qreal animProgress() const { return m_animProgress; }
    void setAnimProgress(qreal progress);
    
    // Sizing
    QSize sizeHint() const override { return QSize(400, 150); }
    QSize minimumSizeHint() const override { return QSize(200, 100); }

signals:
    void extractMinCompleted(double value, int runIndex);
    void animationCompleted();
    void nodeClicked(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void recalculatePositions();
    void drawNode(QPainter& painter, const HeapNode& node, int index);
    void drawEdge(QPainter& painter, int parentIdx, int childIdx);
    QPointF getNodePosition(int index) const;
    int getNodeAtPoint(const QPoint& point) const;
    QColor getRunColor(int runIndex) const;
    void startAnimation(int durationMs = 400);
    
    // Heap data
    QVector<HeapNode> m_nodes;
    QVector<HeapNode> m_targetNodes;  // For animation interpolation
    
    // Visual parameters
    qreal m_nodeRadius = 25;
    qreal m_levelSpacing = 60;
    qreal m_nodeSpacing = 30;
    qreal m_topPadding = 40;
    
    // Animation
    bool m_animating = false;
    qreal m_animProgress = 0.0;
    QPropertyAnimation* m_animation = nullptr;
    int m_highlightedIndex = -1;
    int m_extractingIndex = -1;
    QPair<int, int> m_swappingIndices{-1, -1};
    
    // Colors
    QVector<QColor> m_runColors{
        QColor(70, 130, 200),   // Blue
        QColor(200, 100, 70),   // Red
        QColor(70, 180, 100),   // Green
        QColor(180, 130, 70),   // Orange
        QColor(140, 70, 180),   // Purple
        QColor(70, 180, 180),   // Cyan
        QColor(180, 70, 130),   // Pink
        QColor(130, 180, 70)    // Lime
    };
};
