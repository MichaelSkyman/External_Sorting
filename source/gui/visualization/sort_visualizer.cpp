#include "sort_visualizer.h"
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>

// ========== VisualizerLayout - IMPROVED ==========

void VisualizerLayout::calculate(const QSize& widgetSize, int blockCount)
{
    // Check cache to prevent unnecessary recalculation (reduces jitter)
    if (geometryCached && lastWidgetSize == widgetSize && lastBlockCount == blockCount) {
        return;
    }
    
    qreal w = widgetSize.width();
    qreal h = widgetSize.height();
    
    // Determine layout mode based on window size
    if (h < 500) {
        mode = LayoutMode::Compact;
    } else if (h < 800) {
        mode = LayoutMode::Standard;
    } else {
        mode = LayoutMode::Expanded;
    }
    
    // Calculate block size based on available width and count
    // IMPROVED: Larger blocks with better spacing
    int effectiveCount = qMax(1, blockCount);
    qreal availableWidth = w - 2 * margin;
    blockWidth = calculateBlockWidth(availableWidth, effectiveCount);
    blockHeight = blockWidth * 1.4;  // Slightly less tall for better fit
    
    // Dynamic section spacing based on mode
    switch (mode) {
        case LayoutMode::Compact:
            sectionSpacing = 15;
            break;
        case LayoutMode::Standard:
            sectionSpacing = 25;
            break;
        case LayoutMode::Expanded:
            sectionSpacing = 35;
            break;
    }
    
    // Calculate area rectangles with proper vertical distribution
    // Reserve space for status bar (compact)
    qreal statusHeight = 25;
    qreal availableHeight = h - statusHeight - 2 * margin;
    
    // IMPROVED: Better proportions with visualization taking 75-80% of space
    // Disk area - compact at top (12%)
    qreal diskHeight = availableHeight * 0.12;
    diskArea = QRectF(margin, margin, w - 2 * margin, diskHeight);
    
    // RAM area - DOMINANT, larger area (35%)
    qreal ramTop = diskArea.bottom() + sectionSpacing;
    qreal ramHeight = availableHeight * 0.35;
    ramArea = QRectF(margin, ramTop, w - 2 * margin, ramHeight);
    
    // Runs area - moderate (20%)
    qreal runsTop = ramArea.bottom() + sectionSpacing;
    qreal runsHeight = availableHeight * 0.20;
    runsArea = QRectF(margin, runsTop, w - 2 * margin, runsHeight);
    
    // Output area - remaining space (25%)
    qreal outputTop = runsArea.bottom() + sectionSpacing;
    qreal outputHeight = availableHeight * 0.25;
    outputArea = QRectF(margin, outputTop, w - 2 * margin, outputHeight);
    
    // Status area - very bottom, compact
    statusArea = QRectF(margin, h - statusHeight - margin/2, w - 2 * margin, statusHeight);
    
    // Update cache
    geometryCached = true;
    lastWidgetSize = widgetSize;
    lastBlockCount = blockCount;
}

qreal VisualizerLayout::calculateBlockWidth(qreal availableWidth, int blockCount) const
{
    // Calculate optimal block width with proper spacing
    qreal totalSpacing = spacing * (blockCount + 1);
    qreal maxWidth = (availableWidth - totalSpacing) / blockCount;
    
    // Clamp to reasonable range - ENLARGED minimums
    return qBound(minBlockWidth, maxWidth, maxBlockWidth);
}

// ========== AnimatedArrow - IMPROVED ==========

void AnimatedArrow::render(QPainter* painter) const
{
    if (!visible || opacity < 0.01) return;
    
    painter->save();
    painter->setOpacity(opacity);
    
    // Use eased progress for smooth animation
    double renderProgress = easedProgress();
    
    QPen pen(color, 3);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    
    // Build the path
    QPainterPath path;
    path.moveTo(start);
    
    if (useCurve) {
        path.quadTo(control, end);
    } else {
        path.lineTo(end);
    }
    
    // Draw partial path based on progress
    if (renderProgress < 1.0) {
        qreal length = path.length();
        if (length > 0) {
            // Sample points along path for smooth partial drawing
            QPainterPath trimmedPath;
            trimmedPath.moveTo(start);
            
            int numSegments = qMax(10, int(length / 5));
            for (int i = 1; i <= numSegments; ++i) {
                qreal t = static_cast<qreal>(i) / numSegments;
                if (t <= renderProgress) {
                    trimmedPath.lineTo(path.pointAtPercent(t));
                } else {
                    break;
                }
            }
            
            painter->drawPath(trimmedPath);
        }
    } else {
        painter->drawPath(path);
    }
    
    // Draw arrowhead with smooth fade-in at end
    if (renderProgress > 0.85) {
        double arrowOpacity = (renderProgress - 0.85) / 0.15; // Fade in last 15%
        painter->setOpacity(opacity * arrowOpacity);
        
        QPointF tip = end;
        QPointF tangent;
        
        if (useCurve) {
            qreal t = 0.99;
            tangent = 2 * (1 - t) * (control - start) + 2 * t * (end - control);
        } else {
            tangent = end - start;
        }
        
        qreal angle = qAtan2(tangent.y(), tangent.x());
        qreal arrowSize = 14;  // Slightly larger
        
        QPointF p1 = tip - QPointF(qCos(angle - M_PI/6) * arrowSize,
                                   qSin(angle - M_PI/6) * arrowSize);
        QPointF p2 = tip - QPointF(qCos(angle + M_PI/6) * arrowSize,
                                   qSin(angle + M_PI/6) * arrowSize);
        
        painter->setBrush(color);
        painter->setPen(Qt::NoPen);
        QPolygonF arrow;
        arrow << tip << p1 << p2;
        painter->drawPolygon(arrow);
    }
    
    painter->restore();
}

void AnimatedArrow::animate(double deltaTime, double speed)
{
    // Smooth progress animation
    if (progress < 1.0) {
        progress = qMin(1.0, progress + deltaTime * speed);
    }
    
    // Smooth opacity transitions
    if (qAbs(opacity - targetOpacity) > 0.01) {
        double opacitySpeed = 3.0 * deltaTime; // Smoother fade
        if (targetOpacity > opacity) {
            opacity = qMin(targetOpacity, opacity + opacitySpeed);
        } else {
            opacity = qMax(targetOpacity, opacity - opacitySpeed);
        }
    }
}

// ========== VisualizerLayout adaptive methods - IMPROVED ==========

void VisualizerLayout::calculateAdaptive(const QSize& widgetSize, int blockCount, qreal zoomFactor)
{
    // Invalidate cache when zoom changes
    if (geometryCached) {
        geometryCached = false;
    }
    
    // First do basic calculation
    calculate(widgetSize, blockCount);
    
    // Apply zoom to block dimensions with smooth scaling
    qreal baseWidth = blockWidth;
    blockWidth = qBound(minBlockWidth, baseWidth * zoomFactor, maxBlockWidth);
    blockHeight = blockWidth * 1.4;
    
    // Calculate how many blocks are visible at current zoom
    qreal availableWidth = widgetSize.width() - 2 * margin;
    visibleBlockCount = static_cast<int>(availableWidth / (blockWidth + spacing));
    
    // Re-enable cache after adaptive calculation
    geometryCached = true;
}

bool VisualizerLayout::needsUpdate(const QSize& widgetSize, int blockCount) const
{
    return !geometryCached || lastWidgetSize != widgetSize || lastBlockCount != blockCount;
}

// ========== Viewport - IMPROVED ==========

bool Viewport::isBlockVisible(int index, qreal blockWidth, qreal spacing) const
{
    qreal blockStart = index * (blockWidth + spacing) - scrollOffset;
    qreal blockEnd = blockStart + blockWidth;
    // Add small margin for smoother transitions
    return blockEnd > -blockWidth && blockStart < visibleWidth + blockWidth;
}

QPointF Viewport::adjustPosition(const QPointF& pos) const
{
    return QPointF(pos.x() - scrollOffset, pos.y());
}

// ========== SortVisualizer - IMPROVED ==========

SortVisualizer::SortVisualizer(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(500, 400);  // Larger minimum size
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
    
    // IMPROVED: Better anti-aliasing and rendering hints
    setAttribute(Qt::WA_NoSystemBackground, true);
    
    // Initialize smooth animations with proper easing curves
    cameraAnim = new QPropertyAnimation(this, "cameraOffsetY", this);
    cameraAnim->setEasingCurve(QEasingCurve::InOutCubic);
    cameraAnim->setDuration(m_defaultAnimDuration);
    
    ramScaleAnim = new QPropertyAnimation(this, "ramScale", this);
    ramScaleAnim->setEasingCurve(QEasingCurve::OutBack);
    ramScaleAnim->setDuration(m_defaultAnimDuration);
    
    introAnim = new QPropertyAnimation(this, "introProgress", this);
    introAnim->setEasingCurve(QEasingCurve::OutCubic);
    introAnim->setDuration(m_slowAnimDuration);
    
    // NEW: Highlight animation for smooth selection effects
    m_highlightAnim = new QPropertyAnimation(this, "highlightIntensity", this);
    m_highlightAnim->setEasingCurve(QEasingCurve::InOutSine);
    m_highlightAnim->setDuration(m_fastAnimDuration);
    
    // NEW: Transition animation for state changes
    m_transitionAnim = new QPropertyAnimation(this, "transitionProgress", this);
    m_transitionAnim->setEasingCurve(QEasingCurve::InOutQuad);
    m_transitionAnim->setDuration(m_defaultAnimDuration);
    
    // Initialize color transitions
    m_diskHighlightColor.currentColor = QColor(50, 50, 55);
    m_ramHighlightColor.currentColor = QColor(45, 55, 65);
    
    frameTimer.start();
}

SortVisualizer::~SortVisualizer()
{
}

void SortVisualizer::setController(AnimationController* ctrl)
{
    controller = ctrl;
    
    if (controller) {
        connect(controller, &AnimationController::stepStarted,
                this, &SortVisualizer::onStepStarted);
        connect(controller, &AnimationController::stepProgress,
                this, &SortVisualizer::onStepProgress);
        connect(controller, &AnimationController::stepCompleted,
                this, &SortVisualizer::onStepCompleted);
        connect(controller, &AnimationController::frameUpdate,
                this, &SortVisualizer::onFrameUpdate);
        connect(controller, &AnimationController::playbackStarted,
                this, &SortVisualizer::onPlaybackStarted);
        connect(controller, &AnimationController::playbackStopped,
                this, &SortVisualizer::onPlaybackStopped);
    }
}

void SortVisualizer::setData(const QVector<double>& data)
{
    diskBlocks.clear();
    ramBlocks.clear();
    runBlocks.clear();
    outputBlocks.clear();
    movingBlocks.clear();
    
    totalBlocks = data.size();
    
    // Invalidate layout cache for proper recalculation
    layout.invalidateCache();
    m_geometryDirty = true;
    
    // Create disk blocks
    for (int i = 0; i < data.size(); ++i) {
        MemoryBlock block(data[i], i);
        block.state = BlockState::Disk;
        diskBlocks.append(block);
    }
    
    // Initialize block highlight colors
    m_blockHighlightColors.resize(data.size());
    for (auto& ct : m_blockHighlightColors) {
        ct.currentColor = QColor(100, 100, 110);
    }
    
    recalculateLayout();
    
    // Reset visual state with proper initial values
    m_introProgress = 0.0;
    m_ramScale = 0.8;  // Larger initial scale
    m_cameraOffsetY = 0.0;
    m_highlightIntensity = 0.0;
    m_transitionProgress = 0.0;
    diskHighlighted = false;
    ramHighlighted = false;
    runsGenerated = 0;
    
    emit visualizationReady();
    update();  // Use update() instead of repaint() for smooth rendering
}

void SortVisualizer::clear()
{
    diskBlocks.clear();
    ramBlocks.clear();
    runBlocks.clear();
    outputBlocks.clear();
    movingBlocks.clear();
    arrows.clear();
    m_blockHighlightColors.clear();
    totalBlocks = 0;
    m_geometryDirty = true;
    layout.invalidateCache();
    update();
}

void SortVisualizer::recalculateLayout()
{
    // Use cached layout when possible to prevent jitter
    if (!m_geometryDirty && !layout.needsUpdate(size(), totalBlocks)) {
        return;
    }
    
    layout.calculate(size(), totalBlocks);
    
    // Position disk blocks with proper spacing
    qreal x = layout.diskArea.left() + layout.spacing;
    qreal y = layout.diskArea.center().y() - layout.blockHeight / 2;
    qreal maxX = layout.diskArea.right() - layout.blockWidth;
    
    m_cachedDiskPositions.resize(diskBlocks.size());
    for (int i = 0; i < diskBlocks.size(); ++i) {
        m_cachedDiskPositions[i] = QRectF(x, y, layout.blockWidth, layout.blockHeight);
        diskBlocks[i].rect = m_cachedDiskPositions[i];
        x += layout.blockWidth + layout.spacing;
        
        // Wrap to next row if needed
        if (x > maxX) {
            x = layout.diskArea.left() + layout.spacing;
            y += layout.blockHeight + layout.spacing;
        }
    }
    
    // Position RAM blocks similarly
    x = layout.ramArea.left() + layout.spacing;
    y = layout.ramArea.center().y() - layout.blockHeight / 2;
    
    m_cachedRamPositions.resize(ramBlocks.size());
    for (int i = 0; i < ramBlocks.size(); ++i) {
        m_cachedRamPositions[i] = QRectF(x, y, layout.blockWidth, layout.blockHeight);
        ramBlocks[i].rect = m_cachedRamPositions[i];
        x += layout.blockWidth + layout.spacing;
    }
    
    m_geometryDirty = false;
}

void SortVisualizer::invalidateLayout()
{
    m_geometryDirty = true;
    layout.invalidateCache();
    recalculateLayout();
}

void SortVisualizer::setCameraOffsetY(qreal offset)
{
    if (!qFuzzyCompare(m_cameraOffsetY, offset)) {
        m_cameraOffsetY = offset;
        update();  // Use update() for batched repainting
    }
}

void SortVisualizer::setRamScale(qreal scale)
{
    if (!qFuzzyCompare(m_ramScale, scale)) {
        m_ramScale = scale;
        update();
    }
}

void SortVisualizer::setIntroProgress(qreal progress)
{
    if (!qFuzzyCompare(m_introProgress, progress)) {
        m_introProgress = progress;
        update();
    }
}

void SortVisualizer::setHighlightIntensity(qreal intensity)
{
    if (!qFuzzyCompare(m_highlightIntensity, intensity)) {
        m_highlightIntensity = qBound(0.0, intensity, 1.0);
        update();
    }
}

void SortVisualizer::setTransitionProgress(qreal progress)
{
    if (!qFuzzyCompare(m_transitionProgress, progress)) {
        m_transitionProgress = qBound(0.0, progress, 1.0);
        update();
    }
}

// ========== Zoom and Viewport ==========

void SortVisualizer::setZoomFactor(qreal zoom)
{
    qreal newZoom = qBound(m_minZoom, zoom, m_maxZoom);
    if (qFuzzyCompare(m_zoomFactor, newZoom)) return;
    
    m_zoomFactor = newZoom;
    layout.calculateAdaptive(size(), totalBlocks, m_zoomFactor);
    updateViewport();
    emit zoomChanged(m_zoomFactor);
    update();
}

void SortVisualizer::zoomIn()
{
    setZoomFactor(m_zoomFactor + m_zoomStep);
}

void SortVisualizer::zoomOut()
{
    setZoomFactor(m_zoomFactor - m_zoomStep);
}

void SortVisualizer::resetZoom()
{
    setZoomFactor(1.0);
}

void SortVisualizer::setViewportOffset(qreal offset)
{
    qreal maxOffset = qMax(0.0, m_viewport.totalWidth - m_viewport.visibleWidth);
    m_viewport.scrollOffset = qBound(0.0, offset, maxOffset);
    
    // Calculate visible block range
    int blocksPerRow = qMax(1, static_cast<int>(
        m_viewport.visibleWidth / (layout.blockWidth + layout.spacing)));
    m_viewport.startIndex = static_cast<int>(
        m_viewport.scrollOffset / (layout.blockWidth + layout.spacing));
    m_viewport.endIndex = qMin(totalBlocks, 
        m_viewport.startIndex + blocksPerRow + 2); // +2 for partial blocks
    
    emit viewportChanged(m_viewport.scrollOffset, m_viewport.visibleWidth, m_viewport.totalWidth);
    update();
}

void SortVisualizer::ensureBlockVisible(int blockIndex)
{
    if (blockIndex < 0 || blockIndex >= totalBlocks) return;
    
    qreal blockStart = blockIndex * (layout.blockWidth + layout.spacing);
    qreal blockEnd = blockStart + layout.blockWidth;
    
    // Check if block is already visible
    if (blockStart >= m_viewport.scrollOffset && 
        blockEnd <= m_viewport.scrollOffset + m_viewport.visibleWidth) {
        return;
    }
    
    // Scroll to make block visible (center it if possible)
    qreal targetOffset = blockStart - (m_viewport.visibleWidth - layout.blockWidth) / 2;
    
    // Use smooth scroll animation
    smoothScrollTo(targetOffset, m_defaultAnimDuration);
}

void SortVisualizer::scrollToBlock(int blockIndex)
{
    if (blockIndex < 0 || blockIndex >= totalBlocks) return;
    
    qreal blockStart = blockIndex * (layout.blockWidth + layout.spacing);
    qreal targetOffset = blockStart - layout.margin;
    smoothScrollTo(targetOffset, m_fastAnimDuration);
}

void SortVisualizer::smoothScrollTo(qreal offset, int durationMs)
{
    // Stop any existing scroll animation
    if (m_scrollAnim) {
        m_scrollAnim->stop();
        m_scrollAnim->deleteLater();
    }
    
    m_scrollAnim = new QPropertyAnimation(this, "viewportOffset", this);
    m_scrollAnim->setDuration(durationMs);
    m_scrollAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_scrollAnim->setStartValue(m_viewport.scrollOffset);
    m_scrollAnim->setEndValue(offset);
    m_scrollAnim->start(QAbstractAnimation::DeleteWhenStopped);
    
    m_viewport.isScrolling = true;
    m_viewport.targetOffset = offset;
    
    connect(m_scrollAnim, &QPropertyAnimation::finished, this, [this]() {
        m_viewport.isScrolling = false;
    });
}

void SortVisualizer::updateViewport()
{
    m_viewport.visibleWidth = width() - 2 * layout.margin;
    m_viewport.totalWidth = totalBlocks * (layout.blockWidth + layout.spacing);
    
    // Re-clamp scroll offset
    setViewportOffset(m_viewport.scrollOffset);
}

bool SortVisualizer::shouldRenderBlock(int index) const
{
    return m_viewport.isBlockVisible(index, layout.blockWidth, layout.spacing);
}

QVector<QPair<int, QPointF>> SortVisualizer::exportBlockPositions() const
{
    QVector<QPair<int, QPointF>> positions;
    
    // Export disk blocks
    for (int i = 0; i < diskBlocks.size(); ++i) {
        positions.append({i, getDiskBlockPosition(i)});
    }
    
    // Export RAM blocks
    for (int i = 0; i < ramBlocks.size(); ++i) {
        positions.append({1000 + i, getRAMBlockPosition(i)});  // 1000+ for RAM
    }
    
    return positions;
}

void SortVisualizer::importBlockPositions(const QVector<QPair<int, QPointF>>& positions)
{
    // This would restore block positions from a saved state
    // For now, just trigger layout recalculation
    Q_UNUSED(positions);
    recalculateLayout();
    update();
}

void SortVisualizer::onStepStarted(const AnimationStep& step)
{
    currentStep = step;
    currentProgress = 0.0;
    
    // Handle based on step type
    switch (step.type) {
        case StepType::PhaseTransition:
            showPhaseOverlay = true;
            phaseText = step.statusText;
            phaseOverlayProgress = 0.0;
            break;
            
        case StepType::LoadToRAM:
            if (step.blockIndex >= 0 && step.blockIndex < diskBlocks.size()) {
                // Start moving block animation
                MemoryBlock movingBlock = diskBlocks[step.blockIndex];
                movingBlock.state = BlockState::Loading;
                movingBlock.isAnimating = true;
                
                MovingBlock mb(movingBlock);
                mb.moveTo(getRAMBlockPosition(ramBlocks.size()), step.durationMs);
                movingBlocks.addBlock(movingBlock);
                movingBlocks.blocks.last().moveTo(getRAMBlockPosition(ramBlocks.size()), step.durationMs);
                
                diskBlocks[step.blockIndex].opacity = 0.3;
            }
            break;
            
        case StepType::CameraFocus:
            cameraAnim->stop();
            cameraAnim->setStartValue(m_cameraOffsetY);
            cameraAnim->setEndValue(step.targetPos.y());
            cameraAnim->setDuration(step.durationMs);
            cameraAnim->start();
            break;
            
        case StepType::ZoomRAM:
            ramScaleAnim->stop();
            ramScaleAnim->setStartValue(m_ramScale);
            ramScaleAnim->setEndValue(step.customData.toFloat());
            ramScaleAnim->setDuration(step.durationMs);
            ramScaleAnim->start();
            break;
            
        default:
            break;
    }
    
    update();
}

void SortVisualizer::onStepProgress(const AnimationStep& step, double progress)
{
    currentProgress = progress;
    
    // Execute step-specific progress updates
    switch (step.type) {
        case StepType::IntroDisk:
            executeIntroDisk(step, progress);
            break;
        case StepType::HighlightRAM:
            executeHighlightRAM(step, progress);
            break;
        case StepType::LoadToRAM:
            executeLoadToRAM(step, progress);
            break;
        case StepType::SortBlock:
            executeSortBlock(step, progress);
            break;
        case StepType::WriteToRun:
            executeWriteRun(step, progress);
            break;
        case StepType::MergeStep:
            executeMergeStep(step, progress);
            break;
        case StepType::WriteOutput:
            executeWriteOutput(step, progress);
            break;
        case StepType::PhaseTransition:
            executePhaseTransition(step, progress);
            break;
        default:
            break;
    }
    
    update();
}

void SortVisualizer::onStepCompleted(const AnimationStep& step)
{
    currentProgress = 1.0;
    
    // Finalize step effects
    switch (step.type) {
        case StepType::LoadToRAM:
            // Add block to RAM
            if (!step.values.isEmpty()) {
                for (double val : step.values) {
                    MemoryBlock block(val, ramBlocks.size());
                    block.state = BlockState::InRAM;
                    block.rect = QRectF(getRAMBlockPosition(ramBlocks.size()),
                                        QSizeF(layout.blockWidth, layout.blockHeight));
                    ramBlocks.append(block);
                }
            }
            movingBlocks.clear();
            break;
            
        case StepType::SortBlock:
            // Update RAM blocks with sorted values
            if (!step.values.isEmpty()) {
                for (int i = 0; i < qMin(step.values.size(), ramBlocks.size()); ++i) {
                    ramBlocks[i].value = step.values[i];
                    ramBlocks[i].state = BlockState::InRAM;
                    ramBlocks[i].isAnimating = false;
                }
            }
            break;
            
        case StepType::WriteToRun:
            // Create run visualization
            {
                QVector<MemoryBlock> run;
                for (double val : step.values) {
                    MemoryBlock block(val, run.size());
                    block.state = BlockState::Writing;
                    run.append(block);
                }
                runBlocks.append(run);
                runsGenerated++;
                
                // Clear RAM
                ramBlocks.clear();
            }
            break;
            
        case StepType::WriteOutput:
            // Add to output
            if (!step.values.isEmpty()) {
                MemoryBlock block(step.values[0], outputBlocks.size());
                block.state = BlockState::Output;
                block.rect = QRectF(getOutputBlockPosition(outputBlocks.size()),
                                    QSizeF(layout.blockWidth, layout.blockHeight));
                outputBlocks.append(block);
            }
            break;
            
        case StepType::PhaseTransition:
            showPhaseOverlay = false;
            break;
            
        default:
            break;
    }
    
    // Clear highlights
    highlightedDiskBlock = -1;
    highlightedRAMBlock = -1;
    comparedIndices.clear();
    
    update();
}

void SortVisualizer::onFrameUpdate(double deltaTime, double /*stepProgress*/)
{
    // Prevent excessive updates - batch them
    if (m_pendingUpdate) return;
    m_pendingUpdate = true;
    
    // Update all block animations with smooth interpolation
    updateBlockAnimations(deltaTime);
    
    // Update arrows with smooth progress
    for (auto& arrow : arrows) {
        arrow.animate(deltaTime, 2.0);
    }
    
    // Update moving blocks smoothly
    movingBlocks.updateAll(deltaTime * 1000);
    
    // Update color transitions for smooth highlight effects
    double deltaMsRaw = deltaTime * 1000;
    m_diskHighlightColor.update(deltaMsRaw);
    m_ramHighlightColor.update(deltaMsRaw);
    for (auto& ct : m_blockHighlightColors) {
        ct.update(deltaMsRaw);
    }
    
    // Update phase overlay with smooth transition
    if (showPhaseOverlay) {
        phaseOverlayProgress = qMin(1.0, phaseOverlayProgress + deltaTime * 2.0);
    }
    
    // Schedule single repaint at end of frame
    m_pendingUpdate = false;
    update();  // Use update() instead of repaint() for batched, non-blocking updates
}

void SortVisualizer::onPlaybackStarted()
{
    frameTimer.restart();
}

void SortVisualizer::onPlaybackStopped()
{
}

void SortVisualizer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    
    // IMPROVED: High quality rendering
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    
    // Apply camera offset for smooth scrolling
    painter.translate(0, m_cameraOffsetY);
    
    // Draw all areas in proper order (back to front)
    drawBackground(painter);
    drawDiskArea(painter);
    drawRAMArea(painter);
    drawRunsArea(painter);
    drawOutputArea(painter);
    drawArrows(painter);
    drawMovingBlocks(painter);
    
    // Reset transform for overlays (always on top)
    painter.resetTransform();
    drawStatusArea(painter);
    drawPhaseOverlay(painter);
}

void SortVisualizer::resizeEvent(QResizeEvent*)
{
    // Mark geometry as dirty and recalculate
    m_geometryDirty = true;
    layout.invalidateCache();
    recalculateLayout();
}

void SortVisualizer::mousePressEvent(QMouseEvent* event)
{
    QPointF pos = event->position();
    
    if (layout.diskArea.contains(pos)) {
        emit regionClicked("disk");
    } else if (layout.ramArea.contains(pos)) {
        emit regionClicked("ram");
    } else if (layout.runsArea.contains(pos)) {
        emit regionClicked("runs");
    } else if (layout.outputArea.contains(pos)) {
        emit regionClicked("output");
    }
}

void SortVisualizer::wheelEvent(QWheelEvent* event)
{
    // Check for Ctrl modifier for zoom
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom
        QPoint angleDelta = event->angleDelta();
        if (angleDelta.y() > 0) {
            zoomIn();
        } else if (angleDelta.y() < 0) {
            zoomOut();
        }
        event->accept();
    } else {
        // Horizontal scroll
        QPoint pixelDelta = event->pixelDelta();
        QPoint angleDelta = event->angleDelta();
        
        qreal scrollAmount = 0;
        if (!pixelDelta.isNull()) {
            scrollAmount = -pixelDelta.x();
        } else {
            // angleDelta is in 1/8 degree increments
            scrollAmount = -angleDelta.y() / 8.0 * (layout.blockWidth + layout.spacing);
        }
        
        setViewportOffset(m_viewport.scrollOffset + scrollAmount);
        event->accept();
    }
}

void SortVisualizer::drawBackground(QPainter& painter)
{
    // Dark gradient background - IMPROVED smoother gradient
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(32, 34, 38));
    bg.setColorAt(0.5, QColor(26, 28, 32));
    bg.setColorAt(1, QColor(22, 24, 28));
    painter.fillRect(rect(), bg);
    
    // Subtle grid - only draw if not animating heavily for performance
    if (currentProgress < 0.1 || currentProgress > 0.9) {
        painter.setPen(QPen(QColor(45, 47, 51), 1));
        int gridSize = 50;
        
        // Vertical lines
        for (int x = 0; x < width(); x += gridSize) {
            painter.drawLine(x, 0, x, height());
        }
        // Horizontal lines
        for (int y = 0; y < height(); y += gridSize) {
            painter.drawLine(0, y, width(), y);
        }
    }
}

void SortVisualizer::drawDiskArea(QPainter& painter)
{
    // Get current highlight color (smooth transition)
    QColor areaColor = m_diskHighlightColor.currentColor;
    if (diskHighlighted && !m_diskHighlightColor.isTransitioning()) {
        m_diskHighlightColor.transitionTo(QColor(60, 70, 80), 200);
    } else if (!diskHighlighted && !m_diskHighlightColor.isTransitioning()) {
        m_diskHighlightColor.transitionTo(QColor(50, 50, 55), 200);
    }
    areaColor = m_diskHighlightColor.currentColor;
    
    // Area background with smooth corners
    painter.setBrush(areaColor);
    painter.setPen(QPen(QColor(80, 85, 90), 2));
    painter.drawRoundedRect(layout.diskArea, 10, 10);
    
    // Compact label - left aligned
    painter.setPen(QColor(150, 155, 160));
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(12);  // Slightly smaller for compactness
    painter.setFont(font);
    painter.drawText(layout.diskArea.adjusted(12, 6, 0, 0), Qt::AlignTop | Qt::AlignLeft, "DISK");
    
    // Block count on right (compact)
    QString countText = QString("(%1)").arg(diskBlocks.size());
    painter.setPen(QColor(100, 105, 110));
    font.setPixelSize(10);
    painter.setFont(font);
    painter.drawText(layout.diskArea.adjusted(0, 6, -12, 0), Qt::AlignTop | Qt::AlignRight, countText);
    
    // Draw blocks with intro animation - using cached positions
    double maxVal = getMaxValue();
    for (int i = 0; i < diskBlocks.size(); ++i) {
        MemoryBlock& block = diskBlocks[i];
        
        // Use cached position to prevent jitter
        if (i < m_cachedDiskPositions.size()) {
            block.rect = m_cachedDiskPositions[i];
        }
        
        // Smooth intro fade in
        if (m_introProgress < 1.0) {
            double blockIntroPoint = static_cast<double>(i) / qMax(1, diskBlocks.size());
            // Smooth easing for intro
            double t = qBound(0.0, (m_introProgress - blockIntroPoint) * 2.5, 1.0);
            block.opacity = t * t * (3.0 - 2.0 * t);  // smoothstep
        }
        
        if (highlightedDiskBlock == i) {
            block.isAnimating = true;
        }
        
        block.render(&painter, maxVal);
    }
}

void SortVisualizer::drawRAMArea(QPainter& painter)
{
    painter.save();
    
    // Apply RAM scale animation - IMPROVED smoother scaling
    QPointF ramCenter = layout.ramArea.center();
    painter.translate(ramCenter);
    painter.scale(m_ramScale, m_ramScale);
    painter.translate(-ramCenter);
    
    // Smooth color transition for highlight
    if (ramHighlighted && !m_ramHighlightColor.isTransitioning()) {
        m_ramHighlightColor.transitionTo(QColor(40, 80, 100), 200);
    } else if (!ramHighlighted && !m_ramHighlightColor.isTransitioning()) {
        m_ramHighlightColor.transitionTo(QColor(45, 55, 65), 200);
    }
    QColor areaColor = m_ramHighlightColor.currentColor;
    
    // Area background with gradient for depth
    QLinearGradient bgGrad(layout.ramArea.topLeft(), layout.ramArea.bottomLeft());
    bgGrad.setColorAt(0, areaColor.lighter(110));
    bgGrad.setColorAt(1, areaColor.darker(110));
    
    painter.setBrush(bgGrad);
    painter.setPen(QPen(QColor(0, 143, 150), ramHighlighted ? 3 : 2));
    painter.drawRoundedRect(layout.ramArea, 12, 12);
    
    // Compact label - IMPROVED styling
    painter.setPen(QColor(0, 200, 210));  // Brighter cyan
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(14);
    painter.setFont(font);
    painter.drawText(layout.ramArea.adjusted(15, 10, 0, 0), Qt::AlignTop | Qt::AlignLeft, "RAM BUFFER");
    
    // RAM capacity indicator (compact)
    QString capacityText = QString("Blocks: %1").arg(ramBlocks.size());
    painter.setPen(QColor(100, 160, 170));
    font.setPixelSize(11);
    font.setBold(false);
    painter.setFont(font);
    painter.drawText(layout.ramArea.adjusted(0, 10, -15, 0), Qt::AlignTop | Qt::AlignRight, capacityText);
    
    // Draw RAM blocks with cached positions
    double maxVal = getMaxValue();
    for (int i = 0; i < ramBlocks.size(); ++i) {
        MemoryBlock& block = ramBlocks[i];
        
        // Use cached position
        if (i < m_cachedRamPositions.size()) {
            block.rect = m_cachedRamPositions[i];
        }
        
        // Smooth highlight transition for compared blocks
        if (highlightedRAMBlock == i || comparedIndices.contains(i)) {
            block.isAnimating = true;
            block.state = BlockState::Compared;
            // Enhanced glow for active blocks
            block.glowIntensity = qMin(1.0, block.glowIntensity + 0.1);
        } else {
            block.glowIntensity = qMax(0.0, block.glowIntensity - 0.05);
        }
        
        block.render(&painter, maxVal);
    }
    
    painter.restore();
}

void SortVisualizer::drawRunsArea(QPainter& painter)
{
    // Area background with gradient
    QLinearGradient bgGrad(layout.runsArea.topLeft(), layout.runsArea.bottomLeft());
    bgGrad.setColorAt(0, QColor(50, 55, 50));
    bgGrad.setColorAt(1, QColor(40, 45, 40));
    
    painter.setBrush(bgGrad);
    painter.setPen(QPen(QColor(60, 160, 90), 2));
    painter.drawRoundedRect(layout.runsArea, 10, 10);
    
    // Compact label
    painter.setPen(QColor(80, 200, 120));
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(12);
    painter.setFont(font);
    painter.drawText(layout.runsArea.adjusted(12, 6, 0, 0), Qt::AlignTop | Qt::AlignLeft, 
                    QString("SORTED RUNS (%1)").arg(runsGenerated));
    
    // Calculate proper run box dimensions to prevent overlapping
    qreal availableWidth = layout.runsArea.width() - 2 * layout.spacing;
    qreal availableHeight = layout.runsArea.height() - 35;  // Subtract label space
    int numRuns = qMax(1, runBlocks.size());
    
    // Dynamic run width based on count
    qreal maxRunWidth = 80;
    qreal minRunWidth = 30;
    qreal runSpacing = 8;  // Fixed spacing between runs
    qreal runWidth = qMin(maxRunWidth, (availableWidth - runSpacing * (numRuns - 1)) / numRuns);
    runWidth = qMax(minRunWidth, runWidth);
    
    qreal x = layout.runsArea.left() + layout.spacing;
    qreal y = layout.runsArea.top() + 28;
    qreal runHeight = qMin(availableHeight - 10, 60.0);  // Cap height
    
    for (int r = 0; r < runBlocks.size(); ++r) {
        // Ensure run box doesn't exceed area bounds
        if (x + runWidth > layout.runsArea.right() - layout.spacing) {
            break;  // Don't draw if would overflow
        }
        
        QRectF runRect(x, y, runWidth - 4, runHeight);
        
        // Enhanced gradient
        QLinearGradient runGrad(runRect.topLeft(), runRect.bottomLeft());
        runGrad.setColorAt(0, QColor(70, 140, 90));
        runGrad.setColorAt(0.5, QColor(55, 115, 70));
        runGrad.setColorAt(1, QColor(40, 85, 50));
        
        painter.setBrush(runGrad);
        painter.setPen(QPen(QColor(90, 180, 110), 1));
        painter.drawRoundedRect(runRect, 6, 6);
        
        // Run info - centered
        painter.setPen(Qt::white);
        font.setPixelSize(9);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(runRect, Qt::AlignCenter, 
                        QString("R%1\n%2").arg(r + 1).arg(runBlocks[r].size()));
        
        x += runWidth + runSpacing;
    }
    
    // Show overflow indicator if more runs than can be displayed
    if (runBlocks.size() > 0) {
        int visibleRuns = static_cast<int>((availableWidth + runSpacing) / (runWidth + runSpacing));
        if (runBlocks.size() > visibleRuns) {
            painter.setPen(QColor(150, 155, 160));
            font.setPixelSize(10);
            painter.setFont(font);
            painter.drawText(layout.runsArea.adjusted(0, 0, -10, -5), 
                           Qt::AlignBottom | Qt::AlignRight, 
                           QString("+%1 more").arg(runBlocks.size() - visibleRuns));
        }
    }
}

void SortVisualizer::drawOutputArea(QPainter& painter)
{
    // Area background with gradient
    QLinearGradient bgGrad(layout.outputArea.topLeft(), layout.outputArea.bottomLeft());
    bgGrad.setColorAt(0, QColor(40, 60, 65));
    bgGrad.setColorAt(1, QColor(30, 45, 50));
    
    painter.setBrush(bgGrad);
    painter.setPen(QPen(QColor(0, 160, 170), 2));
    painter.drawRoundedRect(layout.outputArea, 10, 10);
    
    // Compact label
    painter.setPen(QColor(0, 220, 230));
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(12);
    painter.setFont(font);
    painter.drawText(layout.outputArea.adjusted(12, 6, 0, 0), Qt::AlignTop | Qt::AlignLeft, "SORTED OUTPUT");
    
    // Progress indicator with smooth animation
    if (totalBlocks > 0) {
        qreal progress = static_cast<qreal>(outputBlocks.size()) / totalBlocks;
        QString progressText = QString("%1%").arg(static_cast<int>(progress * 100));
        painter.setPen(QColor(0, 200, 210));
        painter.drawText(layout.outputArea.adjusted(0, 6, -12, 0), Qt::AlignTop | Qt::AlignRight, progressText);
        
        // Progress bar - positioned at bottom
        qreal barY = layout.outputArea.bottom() - 18;
        QRectF progressBarBg(layout.outputArea.left() + 12, barY,
                            layout.outputArea.width() - 24, 10);
        
        // Background
        painter.setBrush(QColor(25, 35, 40));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(progressBarBg, 5, 5);
        
        // Fill with gradient
        if (progress > 0) {
            QRectF progressBar = progressBarBg;
            progressBar.setWidth(progressBarBg.width() * progress);
            
            QLinearGradient fillGrad(progressBar.topLeft(), progressBar.topRight());
            fillGrad.setColorAt(0, QColor(0, 140, 150));
            fillGrad.setColorAt(1, QColor(0, 200, 180));
            
            painter.setBrush(fillGrad);
            painter.drawRoundedRect(progressBar, 5, 5);
        }
    }
    
    // Calculate block layout to prevent overlapping
    qreal availableWidth = layout.outputArea.width() - 2 * layout.spacing;
    qreal blockAreaTop = layout.outputArea.top() + 28;
    qreal blockAreaHeight = layout.outputArea.height() - 55;  // Space for label and progress bar
    
    int numBlocks = outputBlocks.size();
    if (numBlocks == 0) return;
    
    // Dynamic block sizing to fit all visible blocks
    qreal minBlockWidth = 8;
    qreal maxBlockWidth = layout.blockWidth * 0.6;
    qreal blockSpacing = 2;
    
    // Calculate how many blocks can fit
    int maxVisible = static_cast<int>((availableWidth + blockSpacing) / (minBlockWidth + blockSpacing));
    int displayCount = qMin(numBlocks, maxVisible);
    int startIdx = qMax(0, numBlocks - displayCount);
    
    // Calculate actual width per block
    qreal blockWidth = (availableWidth - blockSpacing * (displayCount - 1)) / displayCount;
    blockWidth = qBound(minBlockWidth, blockWidth, maxBlockWidth);
    
    // Recalculate display count if block width changed
    displayCount = static_cast<int>((availableWidth + blockSpacing) / (blockWidth + blockSpacing));
    displayCount = qMin(numBlocks, displayCount);
    startIdx = qMax(0, numBlocks - displayCount);
    
    // Draw output blocks
    double maxVal = getMaxValue();
    qreal x = layout.outputArea.left() + layout.spacing;
    qreal y = blockAreaTop + (blockAreaHeight - blockAreaHeight * 0.8) / 2;
    qreal blockHeight = blockAreaHeight * 0.8;
    
    for (int i = startIdx; i < outputBlocks.size(); ++i) {
        if (x + blockWidth > layout.outputArea.right() - layout.spacing) {
            break;  // Don't overflow
        }
        
        MemoryBlock block = outputBlocks[i];
        block.rect = QRectF(x, y, blockWidth - 2, blockHeight);
        block.render(&painter, maxVal);
        
        x += blockWidth + blockSpacing;
    }
}

void SortVisualizer::drawStatusArea(QPainter& painter)
{
    // Compact status bar background
    QLinearGradient bgGrad(layout.statusArea.topLeft(), layout.statusArea.bottomLeft());
    bgGrad.setColorAt(0, QColor(35, 38, 42));
    bgGrad.setColorAt(1, QColor(28, 30, 34));
    painter.fillRect(layout.statusArea, bgGrad);
    
    // Status text - left side
    painter.setPen(QColor(160, 165, 170));
    QFont font = painter.font();
    font.setPixelSize(11);
    painter.setFont(font);
    
    QString status = currentStep.statusText;
    if (status.isEmpty()) {
        status = "Ready";
    }
    // Truncate if too long
    QFontMetrics fm(font);
    int maxWidth = layout.statusArea.width() / 2 - 20;
    status = fm.elidedText(status, Qt::ElideRight, maxWidth);
    
    painter.drawText(layout.statusArea.adjusted(10, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, status);
    
    // Compact stats on right
    QString stats = QString("Blocks: %1 | Runs: %2 | Output: %3")
                   .arg(totalBlocks)
                   .arg(runsGenerated)
                   .arg(outputBlocks.size());
    painter.setPen(QColor(120, 125, 130));
    font.setPixelSize(10);
    painter.setFont(font);
    painter.drawText(layout.statusArea.adjusted(0, 0, -10, 0), Qt::AlignRight | Qt::AlignVCenter, stats);
}

void SortVisualizer::drawArrows(QPainter& painter)
{
    for (const auto& arrow : arrows) {
        arrow.render(&painter);
    }
}

void SortVisualizer::drawPhaseOverlay(QPainter& painter)
{
    if (!showPhaseOverlay || phaseOverlayProgress < 0.01) return;
    
    // Smooth easing for overlay animation
    double easedProgress = phaseOverlayProgress * phaseOverlayProgress * (3.0 - 2.0 * phaseOverlayProgress);
    
    // Semi-transparent overlay with smooth fade
    int alpha = static_cast<int>(160 * easedProgress);
    painter.fillRect(rect(), QColor(0, 0, 0, alpha));
    
    // Phase text with smooth animation
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(36);  // Larger text
    painter.setFont(font);
    
    // Calculate text position with smooth slide-in
    QRectF textRect = rect();
    qreal slideOffset = (1.0 - easedProgress) * 80;  // Slide up from below
    textRect.moveTop(textRect.top() + slideOffset);
    
    // Text shadow for depth
    painter.setPen(QColor(0, 0, 0, static_cast<int>(150 * easedProgress)));
    QRectF shadowRect = textRect;
    shadowRect.adjust(3, 3, 3, 3);
    painter.drawText(shadowRect, Qt::AlignCenter, phaseText);
    
    // Main text with glow effect
    double glowPulse = 0.7 + 0.3 * qSin(phaseOverlayProgress * M_PI * 2);
    QColor textColor = QColor(0, 220, 235);
    textColor.setAlphaF(easedProgress * glowPulse);
    painter.setPen(textColor);
    painter.drawText(textRect, Qt::AlignCenter, phaseText);
    
    // Outer glow
    font.setPixelSize(37);
    painter.setFont(font);
    textColor.setAlphaF(easedProgress * 0.3);
    painter.setPen(textColor);
    painter.drawText(textRect, Qt::AlignCenter, phaseText);
}

void SortVisualizer::drawMovingBlocks(QPainter& painter)
{
    double maxVal = getMaxValue();
    movingBlocks.renderAll(&painter, maxVal);
}

void SortVisualizer::executeIntroDisk(const AnimationStep& /*step*/, double progress)
{
    // Progressive reveal of disk blocks
    m_introProgress = progress;
}

void SortVisualizer::executeHighlightRAM(const AnimationStep& /*step*/, double progress)
{
    ramHighlighted = true;
    
    // Pulse effect
    m_ramScale = 0.6 + 0.1 * qSin(progress * M_PI);
}

void SortVisualizer::executeLoadToRAM(const AnimationStep& /*step*/, double /*progress*/)
{
    // Update moving block positions
    movingBlocks.updateAll(16); // Approximate frame time
}

void SortVisualizer::executeSortBlock(const AnimationStep& step, double progress)
{
    // Animate sorting visualization
    for (auto& block : ramBlocks) {
        block.state = BlockState::Sorting;
        block.isAnimating = true;
    }
    
    // At completion, show sorted state
    if (progress > 0.9 && !step.values.isEmpty()) {
        for (int i = 0; i < qMin(step.values.size(), ramBlocks.size()); ++i) {
            ramBlocks[i].value = step.values[i];
        }
    }
}

void SortVisualizer::executeWriteRun(const AnimationStep& /*step*/, double progress)
{
    // Animate blocks moving to run area
    for (auto& block : ramBlocks) {
        block.state = BlockState::Writing;
        block.opacity = 1.0 - progress * 0.5;
    }
}

void SortVisualizer::executeMergeStep(const AnimationStep& step, double /*progress*/)
{
    comparedIndices = step.indices;
}

void SortVisualizer::executeWriteOutput(const AnimationStep& /*step*/, double /*progress*/)
{
    // Flash effect on output area
}

void SortVisualizer::executeCameraFocus(const AnimationStep& /*step*/, double /*progress*/)
{
    // Handled by property animation
}

void SortVisualizer::executeZoomRAM(const AnimationStep& /*step*/, double /*progress*/)
{
    // Handled by property animation
}

void SortVisualizer::executePhaseTransition(const AnimationStep& /*step*/, double progress)
{
    phaseOverlayProgress = progress;
}

QPointF SortVisualizer::getDiskBlockPosition(int index) const
{
    if (index < 0 || index >= diskBlocks.size()) {
        return layout.diskArea.topLeft();
    }
    return diskBlocks[index].rect.topLeft();
}

QPointF SortVisualizer::getRAMBlockPosition(int index) const
{
    qreal x = layout.ramArea.left() + layout.spacing + 
              index * (layout.blockWidth + layout.spacing);
    qreal y = layout.ramArea.center().y() - layout.blockHeight / 2;
    return QPointF(x, y);
}

QPointF SortVisualizer::getRunBlockPosition(int runIndex, int blockIndex) const
{
    Q_UNUSED(blockIndex)
    qreal runWidth = 60;
    qreal x = layout.runsArea.left() + layout.spacing + runIndex * runWidth;
    qreal y = layout.runsArea.center().y();
    return QPointF(x, y);
}

QPointF SortVisualizer::getOutputBlockPosition(int index) const
{
    qreal x = layout.outputArea.left() + layout.spacing + 
              index * (layout.blockWidth * 0.5 + layout.spacing);
    qreal y = layout.outputArea.top() + 30;
    return QPointF(x, y);
}

double SortVisualizer::getMaxValue() const
{
    double maxVal = 100.0;
    for (const auto& block : diskBlocks) {
        maxVal = qMax(maxVal, block.value);
    }
    for (const auto& block : ramBlocks) {
        maxVal = qMax(maxVal, block.value);
    }
    for (const auto& block : outputBlocks) {
        maxVal = qMax(maxVal, block.value);
    }
    return maxVal;
}

void SortVisualizer::updateBlockAnimations(double deltaTime)
{
    for (auto& block : diskBlocks) {
        block.updateAnimation(deltaTime);
    }
    for (auto& block : ramBlocks) {
        block.updateAnimation(deltaTime);
    }
    for (auto& block : outputBlocks) {
        block.updateAnimation(deltaTime);
    }
}

void SortVisualizer::sortRAMBlocks()
{
    std::sort(ramBlocks.begin(), ramBlocks.end(), 
              [](const MemoryBlock& a, const MemoryBlock& b) {
                  return a.value < b.value;
              });
    
    // Recalculate positions
    for (int i = 0; i < ramBlocks.size(); ++i) {
        ramBlocks[i].rect.moveTopLeft(getRAMBlockPosition(i));
        ramBlocks[i].index = i;
    }
}
