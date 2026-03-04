#include "merge_heap_widget.h"
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainterPath>
#include <QMouseEvent>
#include <QtMath>

MergeHeapWidget::MergeHeapWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    m_animation = new QPropertyAnimation(this, "animProgress", this);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
    
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        m_animating = false;
        m_highlightedIndex = -1;
        m_extractingIndex = -1;
        m_swappingIndices = {-1, -1};
        m_nodes = m_targetNodes;
        emit animationCompleted();
        update();
    });
}

void MergeHeapWidget::setHeap(const QVector<HeapNode>& nodes)
{
    m_nodes = nodes;
    m_targetNodes = nodes;
    recalculatePositions();
    update();
}

void MergeHeapWidget::clear()
{
    m_nodes.clear();
    m_targetNodes.clear();
    m_animating = false;
    m_highlightedIndex = -1;
    update();
}

void MergeHeapWidget::recalculatePositions()
{
    if (m_nodes.isEmpty()) return;
    
    for (int i = 0; i < m_nodes.size(); i++) {
        m_nodes[i].position = getNodePosition(i);
        m_nodes[i].targetPosition = m_nodes[i].position;
        m_nodes[i].color = getRunColor(m_nodes[i].runIndex);
    }
    
    m_targetNodes = m_nodes;
}

QPointF MergeHeapWidget::getNodePosition(int index) const
{
    if (m_nodes.isEmpty()) return QPointF(width() / 2.0, m_topPadding);
    
    // Calculate level and position within level
    int level = static_cast<int>(qFloor(qLn(index + 1) / qLn(2)));
    int levelStart = (1 << level) - 1;
    int posInLevel = index - levelStart;
    int nodesInLevel = 1 << level;
    
    // Calculate x position
    qreal levelWidth = width() - 2 * m_nodeRadius;
    qreal spacing = levelWidth / (nodesInLevel + 1);
    qreal x = m_nodeRadius + spacing * (posInLevel + 1);
    
    // Calculate y position
    qreal y = m_topPadding + level * m_levelSpacing;
    
    return QPointF(x, y);
}

void MergeHeapWidget::setAnimProgress(qreal progress)
{
    m_animProgress = progress;
    
    // Interpolate node positions
    for (int i = 0; i < m_nodes.size() && i < m_targetNodes.size(); i++) {
        QPointF start = m_nodes[i].position;
        QPointF end = m_targetNodes[i].targetPosition;
        m_nodes[i].position = start + (end - start) * progress;
    }
    
    update();
}

void MergeHeapWidget::animateExtractMin()
{
    if (m_nodes.isEmpty()) return;
    
    m_extractingIndex = 0;
    m_highlightedIndex = 0;
    
    // Store the min value info
    double minVal = m_nodes[0].value;
    int minRun = m_nodes[0].runIndex;
    
    // Move last to root
    if (m_nodes.size() > 1) {
        m_targetNodes[0] = m_nodes.last();
        m_targetNodes[0].targetPosition = getNodePosition(0);
    }
    
    // Remove last
    m_targetNodes.removeLast();
    
    startAnimation(500);
    
    // Emit after animation
    QTimer::singleShot(500, this, [this, minVal, minRun]() {
        emit extractMinCompleted(minVal, minRun);
    });
}

void MergeHeapWidget::animateInsert(double value, int runIndex)
{
    HeapNode newNode(value, runIndex);
    newNode.color = getRunColor(runIndex);
    newNode.opacity = 0.0;
    
    // Add at end
    m_nodes.append(newNode);
    m_targetNodes.append(newNode);
    
    int idx = m_nodes.size() - 1;
    m_nodes[idx].position = QPointF(width() / 2.0, height() + 50);  // Start below
    m_targetNodes[idx].targetPosition = getNodePosition(idx);
    m_targetNodes[idx].opacity = 1.0;
    
    m_highlightedIndex = idx;
    startAnimation(400);
}

void MergeHeapWidget::animateSwap(int idx1, int idx2)
{
    if (idx1 < 0 || idx1 >= m_nodes.size() || idx2 < 0 || idx2 >= m_nodes.size()) {
        return;
    }
    
    m_swappingIndices = {idx1, idx2};
    
    // Swap target positions
    m_targetNodes[idx1].targetPosition = getNodePosition(idx2);
    m_targetNodes[idx2].targetPosition = getNodePosition(idx1);
    
    // Also swap in target array
    std::swap(m_targetNodes[idx1], m_targetNodes[idx2]);
    m_targetNodes[idx1].targetPosition = getNodePosition(idx1);
    m_targetNodes[idx2].targetPosition = getNodePosition(idx2);
    
    startAnimation(350);
}

void MergeHeapWidget::animateHeapify(int index)
{
    if (index < 0 || index >= m_nodes.size()) return;
    
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    
    if (left < m_nodes.size() && m_nodes[left].value < m_nodes[smallest].value) {
        smallest = left;
    }
    if (right < m_nodes.size() && m_nodes[right].value < m_nodes[smallest].value) {
        smallest = right;
    }
    
    if (smallest != index) {
        animateSwap(index, smallest);
    }
}

void MergeHeapWidget::highlightMin()
{
    if (!m_nodes.isEmpty()) {
        m_highlightedIndex = 0;
        update();
    }
}

void MergeHeapWidget::clearHighlights()
{
    m_highlightedIndex = -1;
    m_extractingIndex = -1;
    m_swappingIndices = {-1, -1};
    update();
}

void MergeHeapWidget::startAnimation(int durationMs)
{
    m_animating = true;
    m_animProgress = 0.0;
    m_animation->setDuration(durationMs);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->start();
}

void MergeHeapWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(35, 38, 42));
    bg.setColorAt(1, QColor(28, 30, 34));
    painter.fillRect(rect(), bg);
    
    // Title
    painter.setPen(QColor(0, 180, 190));
    QFont titleFont = painter.font();
    titleFont.setBold(true);
    titleFont.setPixelSize(14);
    painter.setFont(titleFont);
    painter.drawText(QRectF(10, 5, width(), 25), Qt::AlignLeft | Qt::AlignTop, 
                    QString("MIN HEAP (%1 runs)").arg(m_nodes.size()));
    
    if (m_nodes.isEmpty()) {
        painter.setPen(QColor(100, 105, 110));
        painter.drawText(rect(), Qt::AlignCenter, "Heap empty - waiting for merge phase");
        return;
    }
    
    // Draw edges first (behind nodes)
    for (int i = 0; i < m_nodes.size(); i++) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        
        if (left < m_nodes.size()) {
            drawEdge(painter, i, left);
        }
        if (right < m_nodes.size()) {
            drawEdge(painter, i, right);
        }
    }
    
    // Draw nodes
    for (int i = 0; i < m_nodes.size(); i++) {
        drawNode(painter, m_nodes[i], i);
    }
}

void MergeHeapWidget::drawEdge(QPainter& painter, int parentIdx, int childIdx)
{
    if (parentIdx >= m_nodes.size() || childIdx >= m_nodes.size()) return;
    
    QPointF p1 = m_nodes[parentIdx].position;
    QPointF p2 = m_nodes[childIdx].position;
    
    // Edge styling
    bool isSwapEdge = (m_swappingIndices.first == parentIdx && m_swappingIndices.second == childIdx) ||
                      (m_swappingIndices.first == childIdx && m_swappingIndices.second == parentIdx);
    
    QColor edgeColor = isSwapEdge ? QColor(255, 180, 100) : QColor(70, 75, 80);
    int edgeWidth = isSwapEdge ? 3 : 2;
    
    painter.setPen(QPen(edgeColor, edgeWidth));
    painter.drawLine(p1, p2);
}

void MergeHeapWidget::drawNode(QPainter& painter, const HeapNode& node, int index)
{
    QPointF center = node.position;
    qreal radius = m_nodeRadius * node.scale;
    
    painter.save();
    painter.setOpacity(node.opacity);
    
    // Determine highlight state
    bool isHighlighted = (index == m_highlightedIndex);
    bool isExtracting = (index == m_extractingIndex);
    bool isSwapping = (index == m_swappingIndices.first || index == m_swappingIndices.second);
    bool isMin = (index == 0);
    
    // Glow effect for highlighted/extracting nodes
    if (isHighlighted || isExtracting) {
        QRadialGradient glow(center, radius * 1.8);
        QColor glowColor = isExtracting ? QColor(255, 100, 100) : QColor(255, 220, 100);
        glow.setColorAt(0, glowColor);
        glow.setColorAt(0.5, QColor(glowColor.red(), glowColor.green(), glowColor.blue(), 100));
        glow.setColorAt(1, Qt::transparent);
        
        painter.setBrush(glow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, radius * 1.8, radius * 1.8);
    }
    
    // Shadow
    QColor shadowColor(0, 0, 0, 80);
    painter.setBrush(shadowColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center + QPointF(3, 3), radius, radius);
    
    // Node body with gradient
    QColor baseColor = node.color;
    if (isMin) {
        baseColor = QColor(0, 180, 150);  // Min node is teal
    }
    if (isSwapping) {
        baseColor = QColor(255, 180, 100);  // Swapping is orange
    }
    
    QRadialGradient gradient(center - QPointF(radius * 0.3, radius * 0.3), radius * 1.5);
    gradient.setColorAt(0, baseColor.lighter(140));
    gradient.setColorAt(0.5, baseColor);
    gradient.setColorAt(1, baseColor.darker(130));
    
    painter.setBrush(gradient);
    painter.setPen(QPen(baseColor.darker(150), 2));
    painter.drawEllipse(center, radius, radius);
    
    // Value text
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(static_cast<int>(radius * 0.6));
    painter.setFont(font);
    
    QString valueText = QString::number(node.value, 'f', 0);
    painter.drawText(QRectF(center.x() - radius, center.y() - radius * 0.3,
                           radius * 2, radius * 0.8), Qt::AlignCenter, valueText);
    
    // Run index label
    font.setPixelSize(static_cast<int>(radius * 0.35));
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(QColor(255, 255, 255, 180));
    QString runText = QString("R%1").arg(node.runIndex + 1);
    painter.drawText(QRectF(center.x() - radius, center.y() + radius * 0.2,
                           radius * 2, radius * 0.5), Qt::AlignCenter, runText);
    
    // Min indicator
    if (isMin) {
        painter.setPen(QPen(QColor(255, 220, 100), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, radius + 5, radius + 5);
    }
    
    painter.restore();
}

void MergeHeapWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    recalculatePositions();
}

void MergeHeapWidget::mousePressEvent(QMouseEvent* event)
{
    int idx = getNodeAtPoint(event->pos());
    if (idx >= 0) {
        emit nodeClicked(idx);
    }
}

int MergeHeapWidget::getNodeAtPoint(const QPoint& point) const
{
    for (int i = 0; i < m_nodes.size(); i++) {
        QPointF center = m_nodes[i].position;
        qreal dist = QLineF(point, center).length();
        if (dist <= m_nodeRadius) {
            return i;
        }
    }
    return -1;
}

QColor MergeHeapWidget::getRunColor(int runIndex) const
{
    if (runIndex < 0) return QColor(100, 100, 110);
    return m_runColors[runIndex % m_runColors.size()];
}
