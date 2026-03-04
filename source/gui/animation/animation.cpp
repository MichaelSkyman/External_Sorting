#include "animation_manager.h"
#include "data_block.h"
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QtMath>

AnimationManager::AnimationManager(QGraphicsScene* scene)
    : scene(scene)
    , statusText(nullptr)
    , ramBox(nullptr)
    , diskBox(nullptr)
    , ramLabel(nullptr)
    , diskLabel(nullptr)
    , playTimer(nullptr)
    , animGroup(nullptr)
    , playing(false)
    , paused(false)
    , playbackSpeed(1.0)
    , baseDelay(200)
{
    // Create status text
    statusText = new QGraphicsTextItem();
    statusText->setDefaultTextColor(Qt::white);
    QFont font("Segoe UI", 11, QFont::Bold);
    statusText->setFont(font);
    statusText->setPos(10, 5);
    scene->addItem(statusText);
    
    // Create RAM/Disk visualization boxes
    ramBox = new QGraphicsRectItem(480, 10, 50, 25);
    ramBox->setBrush(QBrush(QColor(0, 143, 170, 180)));
    ramBox->setPen(QPen(QColor(0, 143, 170), 2));
    scene->addItem(ramBox);
    
    ramLabel = new QGraphicsTextItem("RAM");
    ramLabel->setDefaultTextColor(Qt::white);
    ramLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));
    ramLabel->setPos(490, 12);
    scene->addItem(ramLabel);
    
    diskBox = new QGraphicsRectItem(540, 10, 50, 25);
    diskBox->setBrush(QBrush(QColor(51, 51, 51)));
    diskBox->setPen(QPen(QColor(100, 100, 100), 1));
    scene->addItem(diskBox);
    
    diskLabel = new QGraphicsTextItem("Disk");
    diskLabel->setDefaultTextColor(QColor(150, 150, 150));
    diskLabel->setFont(QFont("Segoe UI", 8));
    diskLabel->setPos(548, 12);
    scene->addItem(diskLabel);
    
    // Create play timer
    playTimer = new QTimer(this);
    connect(playTimer, &QTimer::timeout, this, &AnimationManager::playNext);
    
    // Create animation group
    animGroup = new QParallelAnimationGroup(this);
    connect(animGroup, &QParallelAnimationGroup::finished, 
            this, &AnimationManager::onAnimationFinished);
}

void AnimationManager::clear()
{
    queue.clear();
    for(auto* block : currentBlocks) {
        scene->removeItem(block);
        delete block;
    }
    currentBlocks.clear();
    
    for(auto* arrow : arrows) {
        scene->removeItem(arrow);
        delete arrow;
    }
    arrows.clear();
    
    emit queueSizeChanged(0);
}

void AnimationManager::setPaused(bool p)
{
    paused = p;
    if(!paused && !queue.isEmpty() && !playing) {
        playNext();
    }
}

void AnimationManager::stepForward()
{
    if(!queue.isEmpty()) {
        SortStep step = queue.dequeue();
        emit queueSizeChanged(queue.size());
        playStep(step);
    }
}

void AnimationManager::setSpeed(double speed)
{
    playbackSpeed = speed;
}

void AnimationManager::displayStep(StepType type, const QVector<double>& data, int progress)
{
    SortStep step;
    step.type = type;
    step.data = data;
    step.progress = progress;
    step.inRAM = (type == StepType::READ || type == StepType::SORT);
    playStep(step);
}

void AnimationManager::enqueueStep(const SortStep& step)
{
    queue.enqueue(step);
    emit queueSizeChanged(queue.size());
    
    if(!playing && !paused) {
        playNext();
    }
}

void AnimationManager::playNext()
{
    if(queue.isEmpty() || paused) {
        playing = false;
        return;
    }
    
    playing = true;
    SortStep step = queue.dequeue();
    emit queueSizeChanged(queue.size());
    playStep(step);
    emit stepPlayed(step);
    
    // Schedule next step
    int delay = static_cast<int>(baseDelay / playbackSpeed);
    QTimer::singleShot(delay, this, &AnimationManager::playNext);
}

void AnimationManager::onAnimationFinished()
{
    // Animation complete callback
}

QColor AnimationManager::getColorForType(StepType type)
{
    switch(type) {
        case StepType::READ:    return QColor(0, 143, 170);   // Blue
        case StepType::SORT:    return QColor(255, 165, 0);   // Orange
        case StepType::WRITE:   return QColor(0, 200, 100);   // Green
        case StepType::MERGE:   return QColor(148, 0, 211);   // Purple
        case StepType::COMPARE: return QColor(255, 255, 0);   // Yellow
        case StepType::SWAP:    return QColor(255, 100, 100); // Red
        default: return Qt::cyan;
    }
}

QColor AnimationManager::getRunColor(int runIndex)
{
    static QList<QColor> runColors = {
        QColor(0, 143, 170),
        QColor(0, 200, 100),
        QColor(255, 165, 0),
        QColor(148, 0, 211),
        QColor(255, 100, 100),
        QColor(100, 200, 255)
    };
    return runColors[runIndex % runColors.size()];
}

void AnimationManager::drawDiskRAMVisualization(bool inRAM)
{
    if(inRAM) {
        ramBox->setBrush(QBrush(QColor(0, 143, 170, 220)));
        ramBox->setPen(QPen(QColor(0, 200, 255), 2));
        ramLabel->setDefaultTextColor(Qt::white);
        
        diskBox->setBrush(QBrush(QColor(51, 51, 51)));
        diskBox->setPen(QPen(QColor(100, 100, 100), 1));
        diskLabel->setDefaultTextColor(QColor(100, 100, 100));
    } else {
        ramBox->setBrush(QBrush(QColor(51, 51, 51)));
        ramBox->setPen(QPen(QColor(100, 100, 100), 1));
        ramLabel->setDefaultTextColor(QColor(100, 100, 100));
        
        diskBox->setBrush(QBrush(QColor(0, 200, 100, 220)));
        diskBox->setPen(QPen(QColor(0, 255, 150), 2));
        diskLabel->setDefaultTextColor(Qt::white);
    }
}

void AnimationManager::drawMergeArrows(int leftIdx, int rightIdx, int targetIdx)
{
    // Clear old arrows
    for(auto* arrow : arrows) {
        scene->removeItem(arrow);
        delete arrow;
    }
    arrows.clear();
    
    if(leftIdx < 0 || rightIdx < 0 || targetIdx < 0) return;
    if(leftIdx >= currentBlocks.size() || rightIdx >= currentBlocks.size()) return;
    
    QPen arrowPen(QColor(148, 0, 211), 2);
    
    // Draw arrows from source blocks to target position
    qreal targetX = 10 + targetIdx * 32 + 14;
    qreal targetY = 130;
    
    if(leftIdx < currentBlocks.size()) {
        QPointF from = currentBlocks[leftIdx]->pos() + QPointF(14, 0);
        auto* line = new QGraphicsLineItem(from.x(), from.y(), targetX - 10, targetY);
        line->setPen(arrowPen);
        scene->addItem(line);
        arrows.append(line);
    }
    
    if(rightIdx < currentBlocks.size()) {
        QPointF from = currentBlocks[rightIdx]->pos() + QPointF(14, 0);
        auto* line = new QGraphicsLineItem(from.x(), from.y(), targetX + 10, targetY);
        line->setPen(arrowPen);
        scene->addItem(line);
        arrows.append(line);
    }
}

void AnimationManager::highlightCompare(int idx1, int idx2)
{
    for(int i = 0; i < currentBlocks.size(); ++i) {
        currentBlocks[i]->setHighlighted(i == idx1 || i == idx2);
    }
}

void AnimationManager::animateWrite(int idx)
{
    if(idx >= 0 && idx < currentBlocks.size()) {
        currentBlocks[idx]->setWriteAnimation(true);
        QTimer::singleShot(150, [this, idx]() {
            if(idx < currentBlocks.size()) {
                currentBlocks[idx]->setWriteAnimation(false);
            }
        });
    }
}

void AnimationManager::colorRuns(const QVector<double>& data)
{
    if(data.isEmpty()) return;
    
    int runIndex = 0;
    double prevVal = data[0];
    
    for(int i = 0; i < currentBlocks.size() && i < data.size(); ++i) {
        if(i > 0 && data[i] < prevVal) {
            runIndex++;
        }
        currentBlocks[i]->setColor(getRunColor(runIndex));
        prevVal = data[i];
    }
}

void AnimationManager::playStep(const SortStep& step)
{
    // Clear old blocks
    for(auto* block : currentBlocks) {
        scene->removeItem(block);
        delete block;
    }
    currentBlocks.clear();
    
    // Clear arrows
    for(auto* arrow : arrows) {
        scene->removeItem(arrow);
        delete arrow;
    }
    arrows.clear();
    
    // Update status text
    QString statusStr;
    switch(step.type) {
        case StepType::READ:    statusStr = "Reading from disk..."; break;
        case StepType::SORT:    statusStr = "Sorting in memory..."; break;
        case StepType::WRITE:   statusStr = "Writing to disk..."; break;
        case StepType::MERGE:   statusStr = "Merging chunks..."; break;
        case StepType::COMPARE: statusStr = "Comparing elements..."; break;
        case StepType::SWAP:    statusStr = "Swapping elements..."; break;
    }
    statusText->setPlainText(QString("%1 (%2%)").arg(statusStr).arg(step.progress));
    
    // Update RAM/Disk visualization
    drawDiskRAMVisualization(step.inRAM);
    
    // Draw data blocks
    QColor blockColor = getColorForType(step.type);
    qreal x = 10;
    qreal y = 45;
    qreal maxHeight = 70.0;
    qreal minHeight = 15.0;
    
    // Find max value for scaling
    double maxVal = 1.0;
    for(double v : step.data) {
        if(qAbs(v) > maxVal) maxVal = qAbs(v);
    }
    
    // Create blocks with smooth animation
    animGroup->clear();
    
    for(int i = 0; i < step.data.size(); ++i) {
        double v = step.data[i];
        qreal height = (qAbs(v) / maxVal) * maxHeight;
        height = qMax(height, minHeight);
        
        auto* block = new DataBlock(v, blockColor, height);
        block->setPos(x, y + (maxHeight - height));
        scene->addItem(block);
        currentBlocks.append(block);
        
        // Animate position from above
        QPropertyAnimation* posAnim = new QPropertyAnimation(block, "pos");
        posAnim->setDuration(static_cast<int>(150 / playbackSpeed));
        posAnim->setStartValue(QPointF(x, y - 20));
        posAnim->setEndValue(QPointF(x, y + (maxHeight - height)));
        posAnim->setEasingCurve(QEasingCurve::OutCubic);
        animGroup->addAnimation(posAnim);
        
        x += 32;
        if(x > 450) {
            x = 10;
            y += maxHeight + 20;
        }
    }
    
    // Apply highlights
    if(step.highlightIdx1 >= 0 || step.highlightIdx2 >= 0) {
        highlightCompare(step.highlightIdx1, step.highlightIdx2);
    }
    
    // Draw merge arrows
    if(step.type == StepType::MERGE) {
        drawMergeArrows(step.mergeLeftIdx, step.mergeRightIdx, step.mergeTargetIdx);
    }
    
    // Color runs for sorted data
    if(step.type == StepType::SORT && !step.runBoundaries.isEmpty()) {
        colorRuns(step.data);
    }
    
    // Write animation effect
    if(step.type == StepType::WRITE) {
        for(int i = 0; i < currentBlocks.size(); ++i) {
            QTimer::singleShot(i * 30, [this, i]() {
                animateWrite(i);
            });
        }
    }
    
    animGroup->start();
}
