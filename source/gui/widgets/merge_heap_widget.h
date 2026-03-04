#pragma once

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QVector>
#include <QPointF>
#include <QTimer>

/** @brief A node in the min-heap visualization, representing one merge-run candidate. */
struct HeapNode {
    double  value        = 0.0;   ///< Data value drawn from a sorted run.
    int     runIndex     = -1;    ///< Index of the run this value came from.
    QPointF position;              ///< Current visual position in the widget.
    QPointF targetPosition;        ///< Animation target position.
    qreal   scale        = 1.0;   ///< Current visual scale factor.
    qreal   opacity      = 1.0;   ///< Current opacity (0–1).
    bool    highlighted  = false; ///< True when this node is the current minimum.
    bool    isExtracting = false; ///< True while an extract-min animation is running.
    bool    isSwapping   = false; ///< True while a heap-swap animation is running.
    QColor  color;                 ///< Run-specific color.
    
    HeapNode() = default;
    HeapNode(double val, int run) : value(val), runIndex(run) {}
};

/** @brief Animates and renders the min-heap used during the k-way merge phase. */
class MergeHeapWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal animProgress READ animProgress WRITE setAnimProgress)

public:
    explicit MergeHeapWidget(QWidget* parent = nullptr);
    
    /// @brief Replaces the current heap with @p nodes and recalculates layout.
    void setHeap(const QVector<HeapNode>& nodes);
    /// @brief Clears the heap display.
    void clear();

    /// @brief Animates extraction of the minimum element.
    void animateExtractMin();
    /// @brief Animates insertion of a new value from @p runIndex.
    void animateInsert(double value, int runIndex);
    /// @brief Animates a swap between nodes at @p idx1 and @p idx2.
    void animateSwap(int idx1, int idx2);
    /// @brief Animates a heapify operation rooted at @p index.
    void animateHeapify(int index);
    /// @brief Highlights the current minimum node.
    void highlightMin();
    /// @brief Clears all node highlights.
    void clearHighlights();

    /// @brief Sets the visual radius of each heap node.
    void setNodeRadius(qreal radius)    { m_nodeRadius   = radius;   update(); }
    /// @brief Sets the vertical spacing between tree levels.
    void setLevelSpacing(qreal spacing) { m_levelSpacing = spacing;  recalculatePositions(); }
    /// @brief Sets the horizontal spacing between sibling nodes.
    void setNodeSpacing(qreal spacing)  { m_nodeSpacing  = spacing;  recalculatePositions(); }

    int  heapSize()    const { return m_nodes.size(); } ///< Returns the current heap size.
    bool isAnimating() const { return m_animating; }    ///< Returns true while animating.

    qreal animProgress() const { return m_animProgress; } ///< Returns the current animation progress.
    /// @brief Sets the animation progress property (used by QPropertyAnimation).
    void setAnimProgress(qreal progress);
    
    /// @brief Returns the recommended size for the heap widget.
    QSize sizeHint() const override { return QSize(400, 150); }
    /// @brief Returns the minimum acceptable size for the heap widget.
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
    
    QVector<HeapNode> m_nodes;       ///< Current heap nodes.
    QVector<HeapNode> m_targetNodes; ///< Target node states for animation interpolation.

    qreal m_nodeRadius   = 25; ///< Visual radius of each heap node circle.
    qreal m_levelSpacing = 60; ///< Vertical spacing between tree levels.
    qreal m_nodeSpacing  = 30; ///< Horizontal spacing between sibling nodes.
    qreal m_topPadding   = 40; ///< Top padding before the root node.

    bool                m_animating       = false;   ///< True while a node animation is running.
    qreal               m_animProgress    = 0.0;     ///< Animation progress (0–1).
    QPropertyAnimation* m_animation       = nullptr; ///< Active property animation (or null).
    int                 m_highlightedIndex = -1;     ///< Index of the highlighted node (-1 = none).
    int                 m_extractingIndex  = -1;     ///< Index of the node being extracted (-1 = none).
    QPair<int, int> m_swappingIndices{-1, -1};
    
    QVector<QColor> m_runColors{ ///< Per-run color palette for node visualization.
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
