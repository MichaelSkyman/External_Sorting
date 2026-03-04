#include "external_sort_canvas.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>

// ====================================================================
//  ANIMATION ENGINE  (unchanged — drives 60 fps fixed-timestep)
// ====================================================================

AnimationEngine::AnimationEngine(QObject* parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->setInterval(static_cast<int>(FRAME_TIME_MS));
    connect(m_timer, &QTimer::timeout, this, &AnimationEngine::onTimerTick);
    m_clock.start();
}

AnimationEngine::~AnimationEngine() { stop(); }

void AnimationEngine::start()
{
    if (m_running) return;
    m_running = true;
    m_paused  = false;
    m_lastFrameTime = m_clock.elapsed();
    m_accumulator   = 0.0;
    m_timer->start();
    emit playbackStarted();
    if (!m_stepQueue.isEmpty()) advanceStep();
}

void AnimationEngine::stop()
{
    m_running = false;
    m_paused  = false;
    m_timer->stop();
    emit playbackStopped();
}

void AnimationEngine::pause()  { m_paused = true; }

void AnimationEngine::resume()
{
    if (!m_running) { start(); return; }
    m_paused = false;
    m_lastFrameTime = m_clock.elapsed();
}

void AnimationEngine::enqueueStep(const AggregatedStep& s)
{
    m_stepQueue.enqueue(s);
    m_allSteps.append(s);
    emit queueSizeChanged(m_stepQueue.size());
}

void AnimationEngine::enqueueSteps(const QVector<AggregatedStep>& steps)
{
    for (const auto& s : steps) {
        m_stepQueue.enqueue(s);
        m_allSteps.append(s);
    }
    emit queueSizeChanged(m_stepQueue.size());
}

void AnimationEngine::clearQueue()
{
    m_stepQueue.clear();
    m_allSteps.clear();
    m_currentStepIndex = -1;
    m_executedSteps = 0;
    emit queueSizeChanged(0);
}

void AnimationEngine::stepForward()
{
    if (m_currentStepIndex < m_allSteps.size() - 1) {
        m_currentStepIndex++;
        m_currentStep   = m_allSteps[m_currentStepIndex];
        m_stepProgress   = 0.0;
        m_stepElapsed    = 0.0;
        m_stepDuration   = calculateDuration(m_currentStep);
        emit stepStarted(m_currentStep);
        emit stepIndexChanged(m_currentStepIndex, m_allSteps.size());
    }
}

void AnimationEngine::stepBackward()
{
    if (m_currentStepIndex > 0) {
        m_currentStepIndex--;
        m_currentStep   = m_allSteps[m_currentStepIndex];
        m_stepProgress   = 0.0;
        m_stepElapsed    = 0.0;
        m_stepDuration   = calculateDuration(m_currentStep);
        emit stepStarted(m_currentStep);
        emit stepIndexChanged(m_currentStepIndex, m_allSteps.size());
    }
}

void AnimationEngine::seekToStep(int idx)
{
    if (idx >= 0 && idx < m_allSteps.size()) {
        m_currentStepIndex = idx;
        m_currentStep   = m_allSteps[idx];
        m_stepProgress   = 0.0;
        m_stepElapsed    = 0.0;
        m_stepDuration   = calculateDuration(m_currentStep);
        emit stepStarted(m_currentStep);
        emit stepIndexChanged(m_currentStepIndex, m_allSteps.size());
    }
}

void AnimationEngine::onTimerTick()
{
    if (m_paused) return;

    qint64 now = m_clock.elapsed();
    double rawDelta = static_cast<double>(now - m_lastFrameTime);
    m_lastFrameTime = now;
    rawDelta = std::min(rawDelta, FRAME_TIME_MS * 4.0);
    m_accumulator += rawDelta;

    while (m_accumulator >= FRAME_TIME_MS) {
        m_deltaTime = FRAME_TIME_MS / 1000.0 * m_speed;
        m_accumulator -= FRAME_TIME_MS;

        if (m_currentStep.type != AggregatedStep::Type::None) {
            m_stepElapsed += FRAME_TIME_MS * m_speed;
            m_stepProgress = std::min(1.0, m_stepElapsed / m_stepDuration);
            if (m_stepProgress >= 1.0) {
                emit stepCompleted(m_currentStep);
                m_executedSteps++;
                advanceStep();
            }
        } else if (!m_stepQueue.isEmpty()) {
            advanceStep();
        }
    }

    emit frameUpdate(m_deltaTime, m_currentStep, m_stepProgress);
}

void AnimationEngine::advanceStep()
{
    if (m_stepQueue.isEmpty()) {
        m_currentStep = AggregatedStep();
        emit queueEmpty();
        return;
    }
    m_currentStep      = m_stepQueue.dequeue();
    m_currentStepIndex = m_allSteps.size() - m_stepQueue.size() - 1;
    m_stepProgress = 0.0;
    m_stepElapsed  = 0.0;
    m_stepDuration = calculateDuration(m_currentStep);

    emit stepStarted(m_currentStep);
    emit stepIndexChanged(m_currentStepIndex, m_allSteps.size());
    emit queueSizeChanged(m_stepQueue.size());
}

double AnimationEngine::calculateDuration(const AggregatedStep& step) const
{
    return step.duration * adaptivePacing() / m_speed;
}

double AnimationEngine::adaptivePacing() const
{
    if (m_executedSteps < 5)   return 2.5;
    if (m_executedSteps < 15)  return 1.8;
    if (m_executedSteps < 40)  return 1.2;
    if (m_executedSteps < 100) return 1.0;
    return 0.7;
}

// ====================================================================
//  BLOCK ITEM — retained-mode scene object for one data block
// ====================================================================

BlockItem::BlockItem(double value, const QColor& color,
                     qreal w, qreal h, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_value(value), m_color(color), m_w(w), m_h(h)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);   // GPU-friendly
    setZValue(10);                                         // above backgrounds
}

QRectF BlockItem::boundingRect() const
{
    // Must encompass glow halo + scaled block
    qreal sw = m_w * m_scale;
    qreal sh = m_h * m_scale;
    qreal glowR = sw * 0.9 + 2;
    qreal ex = std::max(sw / 2, glowR) + 2;
    qreal ey = std::max(sh / 2, glowR) + 2;
    return QRectF(-ex, -ey, ex * 2, ey * 2);
}

void BlockItem::paint(QPainter* painter,
                      const QStyleOptionGraphicsItem* /*option*/,
                      QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    qreal sw = m_w * m_scale;
    qreal sh = m_h * m_scale;
    QRectF r(-sw / 2, -sh / 2, sw, sh);

    painter->setOpacity(m_opacity);

    // ---- Glow halo ----
    if (m_glow > 0.01) {
        qreal radius = sw * 0.9;
        QRadialGradient glow(QPointF(0, 0), radius);
        QColor gc = m_glowColor;
        gc.setAlphaF(std::clamp(0.5 * m_glow, 0.0, 1.0));
        glow.setColorAt(0.0, gc);
        glow.setColorAt(0.6, gc.darker(150));
        glow.setColorAt(1.0, Qt::transparent);
        painter->setPen(Qt::NoPen);
        painter->setBrush(glow);
        painter->drawEllipse(QPointF(0, 0), radius, radius);
    }

    // ---- Block body ----
    QLinearGradient g(r.topLeft(), r.bottomLeft());
    g.setColorAt(0.0, m_color.lighter(125));
    g.setColorAt(0.4, m_color);
    g.setColorAt(1.0, m_color.darker(140));

    painter->setPen(QPen(m_color.darker(160), 1.5));
    painter->setBrush(g);
    painter->drawRoundedRect(r, 5, 5);

    // ---- Value text ----
    painter->setPen(Qt::white);
    QFont f;
    f.setPixelSize(std::max(9, static_cast<int>(m_h * 0.22 * m_scale)));
    f.setBold(true);
    painter->setFont(f);
    painter->drawText(r, Qt::AlignCenter, QString::number(m_value, 'f', 0));

    // ---- Sorted indicator (green underline) ----
    if (m_sorted) {
        painter->setPen(QPen(QColor(0, 220, 100), 2));
        painter->drawLine(r.bottomLeft() + QPointF(4, -3),
                          r.bottomRight() + QPointF(-4, -3));
    }
}

void BlockItem::setBlockOpacity(qreal o)
{
    if (!qFuzzyCompare(m_opacity, o)) {
        m_opacity = o;
        update();
    }
}

void BlockItem::setBlockScale(qreal s)
{
    if (!qFuzzyCompare(m_scale, s)) {
        prepareGeometryChange();
        m_scale = s;
        update();
    }
}

void BlockItem::setGlowIntensity(qreal g)
{
    if (!qFuzzyCompare(m_glow, g)) {
        m_glow = g;
        update();
    }
}

void BlockItem::setBlockSize(qreal w, qreal h)
{
    prepareGeometryChange();
    m_w = w;
    m_h = h;
    update();
}

// ====================================================================
//  CANVAS LAYOUT  (now with guaranteed OUTPUT minimum height)
// ====================================================================

void CanvasLayout::calculate(const QSize& sz, int visCount)
{
    if (isValid && cachedSize == sz && cachedVisCount == visCount) return;

    qreal w = sz.width();
    qreal h = sz.height();

    qreal totalH = h - 2 * margin - 3 * sectionGap;

    // Desired heights
    qreal diskH   = totalH * DISK_H;
    qreal runsH   = totalH * RUNS_H;
    qreal outputH = totalH * OUTPUT_H;
    qreal ramH;

    // Guarantee OUTPUT minimum height; steal from RAM if needed
    if (outputH < OUTPUT_MIN_H && totalH > OUTPUT_MIN_H + 100) {
        outputH = OUTPUT_MIN_H;
        qreal remaining = totalH - diskH - runsH - outputH;
        ramH = std::max(120.0, remaining);
    } else {
        ramH = totalH * RAM_H;
    }

    calculateBlockSize(w - 2 * margin, ramH, std::max(1, visCount));

    qreal y = margin;

    diskArea   = QRectF(margin, y, w - 2 * margin, diskH);
    y += diskH + sectionGap;

    ramArea    = QRectF(margin, y, w - 2 * margin, ramH);
    y += ramH + sectionGap;

    runsArea   = QRectF(margin, y, w - 2 * margin, runsH);
    y += runsH + sectionGap;

    outputArea = QRectF(margin, y, w - 2 * margin, outputH);

    // Initialize output sizing for current count
    recalcOutput(cachedOutCount);

    cachedSize     = sz;
    cachedVisCount = visCount;
    isValid        = true;
}

void CanvasLayout::recalcOutput(int outputCount)
{
    cachedOutCount = outputCount;

    qreal availW = outputArea.width() - 2 * spacing;
    qreal availH = outputArea.height() - 48;  // 40 top + 8 bottom margin

    // Compute how many blocks can fit in rows
    // First, find adaptive width based on count
    if (outputCount <= 0) {
        outputBlockW  = blockWidth;
        outputBlockH  = blockHeight;
        outputCapacity = 20;
        densityMode   = false;
        return;
    }

    // Determine how many rows fit in the available height
    int maxRows = std::max(1, static_cast<int>(availH / (20 + spacing)));

    // Total slots must fill visible area: cols * rows >= outputCount (or capacity)
    // Start with ideal: fit all outputCount blocks
    int targetCount = outputCount;
    int cols = std::max(1, (targetCount + maxRows - 1) / maxRows);
    qreal candidateW = (availW - spacing * (cols + 1)) / cols;

    // Clamp
    candidateW = std::clamp(candidateW, OUT_BLOCK_MIN, OUT_BLOCK_MAX);

    // Recompute cols with clamped width
    cols = std::max(1, static_cast<int>((availW - spacing) / (candidateW + spacing)));
    qreal candidateH = candidateW * 1.3;
    int rows = std::max(1, static_cast<int>((availH) / (candidateH + spacing)));

    outputBlockW  = candidateW;
    outputBlockH  = candidateH;
    outputCapacity = cols * rows;
    densityMode   = (outputBlockW < DENSITY_THRESHOLD);
}

void CanvasLayout::calculateBlockSize(qreal availW, qreal availH, int visCount)
{
    int perRow = std::min(std::max(visCount, 1), maxBlocksPerRow);
    qreal totalSpW = spacing * (perRow + 1);
    qreal optimalW = (availW - totalSpW) / perRow;

    int numRows = (visCount + perRow - 1) / perRow;
    qreal totalSpH = 32 + spacing * (numRows + 1);
    qreal optimalH = numRows > 0
                         ? (availH - totalSpH) / numRows / 1.3
                         : optimalW;

    qreal optimal = std::min(optimalW, optimalH);

    qreal minW = 36.0, maxW = 160.0;
    if (visCount <= 8)       minW = 70.0;
    else if (visCount <= 16) minW = 55.0;
    else if (visCount <= 32) minW = 42.0;

    blockWidth  = std::clamp(optimal, minW, maxW);
    blockHeight = blockWidth * 1.3;
}

QPointF CanvasLayout::ramBlockPos(int index) const
{
    int perRow = std::max(1, static_cast<int>(
        (ramArea.width() - spacing) / (blockWidth + spacing)));
    int row = index / perRow;
    int col = index % perRow;
    qreal x = ramArea.left() + spacing + col * (blockWidth + spacing) + blockWidth / 2;
    qreal y = ramArea.top() + 32 + row * (blockHeight + spacing) + blockHeight / 2;
    return QPointF(std::round(x), std::round(y));
}

QPointF CanvasLayout::outputBlockPos(int index) const
{
    // Adaptive: use outputBlockW/H instead of blockWidth/Height
    int perRow = std::max(1, static_cast<int>(
        (outputArea.width() - spacing) / (outputBlockW + spacing)));
    int row = index / perRow;
    int col = index % perRow;
    qreal x = outputArea.left() + spacing + col * (outputBlockW + spacing) + outputBlockW / 2;
    qreal y = outputArea.top() + 40 + row * (outputBlockH + spacing) + outputBlockH / 2;
    return QPointF(std::round(x), std::round(y));
}

// ====================================================================
//  EXTERNAL SORT CANVAS — QGraphicsView constructor / data / engine
// ====================================================================

ExternalSortCanvas::ExternalSortCanvas(QWidget* parent)
    : QGraphicsView(parent)
{
    // ---- Scene ----
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    // ---- Performance / rendering hints ----
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing
                         | QGraphicsView::DontSavePainterState);

    // ---- Visual settings ----
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(Qt::NoBrush); // we paint gradient in drawBackground

    setMinimumSize(100, 100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ---- Overlay animation ----
    m_overlayAnim = new QPropertyAnimation(this, "overlayOpacity", this);
    m_overlayAnim->setEasingCurve(QEasingCurve::InOutQuad);
    m_overlayAnim->setDuration(400);
}

ExternalSortCanvas::~ExternalSortCanvas() {}

// ====================================================================
//  DATA / ENGINE
// ====================================================================

void ExternalSortCanvas::setData(const QVector<double>& data)
{
    m_totalElements = data.size();
    if (!data.isEmpty()) {
        m_dataMin = *std::min_element(data.begin(), data.end());
        m_dataMax = *std::max_element(data.begin(), data.end());
    }
    m_diskLoaded         = 0;
    clearRamBlocks();
    clearOutputBlocks();
    m_allOutputValues.clear();
    m_runs.clear();
    m_totalOutputWritten = 0;
    m_runsGenerated      = 0;
    m_inMergePhase       = false;
    m_selectedMinRun     = -1;

    m_layout.invalidate();
    recalculateLayout();

    emit visualizationReady();
    invalidateBackground();
}

void ExternalSortCanvas::clear()
{
    m_totalElements      = 0;
    clearRamBlocks();
    clearOutputBlocks();
    m_allOutputValues.clear();
    m_runs.clear();
    m_totalOutputWritten = 0;
    m_runsGenerated      = 0;
    m_inMergePhase       = false;
    invalidateBackground();
}

void ExternalSortCanvas::setEngine(AnimationEngine* engine)
{
    if (m_engine) disconnect(m_engine, nullptr, this, nullptr);
    m_engine = engine;
    if (m_engine) {
        connect(m_engine, &AnimationEngine::frameUpdate,
                this, &ExternalSortCanvas::onFrameUpdate);
        connect(m_engine, &AnimationEngine::stepStarted,
                this, &ExternalSortCanvas::onStepStarted);
        connect(m_engine, &AnimationEngine::stepCompleted,
                this, &ExternalSortCanvas::onStepCompleted);
    }
}

// ====================================================================
//  ZOOM
// ====================================================================

void ExternalSortCanvas::zoomIn()
{
    m_zoom = std::min(m_zoom + 0.25, 3.0);
    resetTransform();
    scale(m_zoom, m_zoom);
    m_layout.invalidate();
    recalculateLayout();
    emit zoomChanged(m_zoom);
}

void ExternalSortCanvas::zoomOut()
{
    m_zoom = std::max(m_zoom - 0.25, 0.5);
    resetTransform();
    scale(m_zoom, m_zoom);
    m_layout.invalidate();
    recalculateLayout();
    emit zoomChanged(m_zoom);
}

void ExternalSortCanvas::resetZoom()
{
    m_zoom = 1.0;
    resetTransform();
    m_layout.invalidate();
    recalculateLayout();
    emit zoomChanged(m_zoom);
}

// ====================================================================
//  PHASE OVERLAY
// ====================================================================

void ExternalSortCanvas::showPhaseOverlay(const QString& text, int ms)
{
    m_overlayText = text;
    m_overlayAnim->stop();
    m_overlayAnim->setStartValue(0.0);
    m_overlayAnim->setEndValue(1.0);
    m_overlayAnim->setDuration(400);
    m_overlayAnim->start();
    QTimer::singleShot(ms, this, &ExternalSortCanvas::hidePhaseOverlay);
}

void ExternalSortCanvas::hidePhaseOverlay()
{
    m_overlayAnim->stop();
    m_overlayAnim->setStartValue(m_overlayOpacity);
    m_overlayAnim->setEndValue(0.0);
    m_overlayAnim->setDuration(300);
    m_overlayAnim->start();
}

void ExternalSortCanvas::setOverlayOpacity(qreal o)
{
    if (!qFuzzyCompare(m_overlayOpacity, o)) {
        m_overlayOpacity = o;
        viewport()->update();  // foreground needs repaint
    }
}

// ====================================================================
//  SCENE MANAGEMENT
// ====================================================================

BlockItem* ExternalSortCanvas::createBlock(double value, const QPointF& pos)
{
    QColor color = valueToColor(value);
    auto* block = new BlockItem(value, color,
                                m_layout.blockWidth, m_layout.blockHeight);
    block->setSnappedPos(pos);
    m_scene->addItem(block);
    return block;
}

void ExternalSortCanvas::clearRamBlocks()
{
    for (auto* b : m_ramBlocks) {
        m_scene->removeItem(b);
        delete b;
    }
    m_ramBlocks.clear();
}

void ExternalSortCanvas::clearOutputBlocks()
{
    for (auto* b : m_outputBlocks) {
        m_scene->removeItem(b);
        delete b;
    }
    m_outputBlocks.clear();
}

void ExternalSortCanvas::invalidateBackground()
{
    resetCachedContent();    // invalidates drawBackground cache
    viewport()->update();
}

// ====================================================================
//  FRAME HANDLER
// ====================================================================

void ExternalSortCanvas::onFrameUpdate(double /*dt*/,
                                        const AggregatedStep& step,
                                        double progress)
{
    m_currentStep  = step;
    m_stepProgress = progress;

    switch (step.type) {
    case AggregatedStep::Type::LoadChunk:       executeLoadChunk(step, progress);       break;
    case AggregatedStep::Type::SortInRAM:       executeSortInRAM(step, progress);       break;
    case AggregatedStep::Type::WriteRun:        executeWriteRun(step, progress);        break;
    case AggregatedStep::Type::MergeStep:       executeMergeStep(step, progress);       break;
    case AggregatedStep::Type::WriteOutput:     executeWriteOutput(step, progress);     break;
    case AggregatedStep::Type::PhaseTransition: executePhaseTransition(step, progress); break;
    default: break;
    }
}

// ====================================================================
//  STEP STARTED
// ====================================================================

void ExternalSortCanvas::onStepStarted(const AggregatedStep& step)
{
    m_currentStep  = step;
    m_stepProgress = 0.0;

    switch (step.type) {

    case AggregatedStep::Type::LoadChunk: {
        clearRamBlocks();
        QVector<double> vis = sampleValues(step.values, MAX_VIS_BLOCKS);

        m_layout.invalidate();
        m_layout.calculate(size(), vis.size());

        for (int i = 0; i < vis.size(); ++i) {
            // Start from disk area, fly to RAM position
            QPointF startPos(m_layout.ramBlockPos(i).x(),
                             m_layout.diskArea.bottom());
            QPointF endPos = m_layout.ramBlockPos(i);

            BlockItem* b = createBlock(vis[i], startPos);
            b->setBlockOpacity(0.0);
            b->setBlockScale(0.7);
            b->setGlowIntensity(0.8);
            b->setGlowColor(QColor(0, 200, 220));

            // Animate position with QPropertyAnimation
            auto* posAnim = new QPropertyAnimation(b, "pos", b);
            posAnim->setDuration(500);
            posAnim->setStartValue(startPos);
            posAnim->setEndValue(endPos);
            posAnim->setEasingCurve(QEasingCurve::InOutCubic);
            posAnim->start(QAbstractAnimation::DeleteWhenStopped);

            // Animate opacity
            auto* opAnim = new QPropertyAnimation(b, "blockOpacity", b);
            opAnim->setDuration(400);
            opAnim->setStartValue(0.0);
            opAnim->setEndValue(1.0);
            opAnim->setEasingCurve(QEasingCurve::OutCubic);
            opAnim->start(QAbstractAnimation::DeleteWhenStopped);

            // Animate scale
            auto* scaleAnim = new QPropertyAnimation(b, "blockScale", b);
            scaleAnim->setDuration(500);
            scaleAnim->setStartValue(0.7);
            scaleAnim->setEndValue(1.0);
            scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
            scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

            // Animate glow fade-out
            auto* glowAnim = new QPropertyAnimation(b, "glowIntensity", b);
            glowAnim->setDuration(600);
            glowAnim->setStartValue(0.8);
            glowAnim->setEndValue(0.0);
            glowAnim->setEasingCurve(QEasingCurve::OutCubic);
            glowAnim->start(QAbstractAnimation::DeleteWhenStopped);

            m_ramBlocks.append(b);
        }
        m_diskLoaded += step.values.size();
        invalidateBackground();
        break;
    }

    case AggregatedStep::Type::SortInRAM: {
        for (auto* b : m_ramBlocks) {
            b->setGlowColor(QColor(255, 200, 50));
            b->setGlowIntensity(0.6);
        }
        break;
    }

    case AggregatedStep::Type::WriteRun:
        break;  // per-frame in executeWriteRun

    case AggregatedStep::Type::MergeStep: {
        m_inMergePhase   = true;
        m_selectedMinRun = step.mergeMinRun;

        for (auto& r : m_runs) { r.active = false; r.highlighted = false; }
        for (int idx : step.sourceBlocks) {
            if (idx >= 0 && idx < m_runs.size()) {
                m_runs[idx].active = true;
                if (idx == step.mergeMinRun)
                    m_runs[idx].highlighted = true;
            }
        }
        invalidateBackground();   // run pills changed
        break;
    }

    case AggregatedStep::Type::PhaseTransition: {
        showPhaseOverlay(step.statusText, static_cast<int>(step.duration * 0.8));

        if (step.statusText.contains("Merge", Qt::CaseInsensitive)) {
            m_inMergePhase = true;
            clearRamBlocks();
            int visRunCount = std::min(static_cast<int>(m_runs.size()),
                                       MAX_VIS_BLOCKS);
            m_layout.invalidate();
            m_layout.calculate(size(), std::max(1, visRunCount));

            for (int i = 0; i < visRunCount; ++i) {
                if (!m_runs[i].hasMore()) continue;
                QPointF pos = m_layout.ramBlockPos(m_ramBlocks.size());
                BlockItem* b = createBlock(m_runs[i].headValue(), pos);
                b->setBlockOpacity(1.0);
                b->setBlockScale(1.0);
                m_ramBlocks.append(b);
            }
        }
        invalidateBackground();
        break;
    }

    case AggregatedStep::Type::Complete:
        showPhaseOverlay("Sorting Complete!", 3000);
        break;

    default: break;
    }
}

// ====================================================================
//  STEP COMPLETED
// ====================================================================

void ExternalSortCanvas::onStepCompleted(const AggregatedStep& step)
{
    switch (step.type) {

    case AggregatedStep::Type::SortInRAM: {
        // Collect values into a sortable list
        struct ValBlock { double v; BlockItem* b; };
        QVector<ValBlock> vb;
        for (auto* b : m_ramBlocks)
            vb.append({b->value(), b});
        std::sort(vb.begin(), vb.end(),
                  [](const ValBlock& a, const ValBlock& b){ return a.v < b.v; });

        // Rebuild m_ramBlocks in sorted order and animate to new positions
        m_ramBlocks.clear();
        for (int i = 0; i < vb.size(); ++i) {
            BlockItem* b = vb[i].b;
            m_ramBlocks.append(b);
            b->setSorted(true);
            b->setGlowColor(QColor(0, 220, 100));
            b->setGlowIntensity(0.6);

            QPointF target = m_layout.ramBlockPos(i);
            auto* anim = new QPropertyAnimation(b, "pos", b);
            anim->setDuration(400);
            anim->setStartValue(b->pos());
            anim->setEndValue(target);
            anim->setEasingCurve(QEasingCurve::InOutCubic);
            anim->start(QAbstractAnimation::DeleteWhenStopped);

            // Fade glow out
            auto* glowAnim = new QPropertyAnimation(b, "glowIntensity", b);
            glowAnim->setDuration(400);
            glowAnim->setStartValue(0.6);
            glowAnim->setEndValue(0.0);
            glowAnim->setEasingCurve(QEasingCurve::OutCubic);
            glowAnim->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
    }

    case AggregatedStep::Type::WriteRun: {
        SortRun run;
        run.index     = m_runsGenerated;
        run.totalSize = step.values.size();
        QVector<double> sorted = step.values;
        std::sort(sorted.begin(), sorted.end());
        run.values = sampleValues(sorted, 200);
        m_runs.append(run);
        m_runsGenerated++;
        clearRamBlocks();
        invalidateBackground();
        break;
    }

    case AggregatedStep::Type::MergeStep: {
        // Advance run pointer
        if (step.mergeMinRun >= 0 && step.mergeMinRun < m_runs.size()) {
            m_runs[step.mergeMinRun].readPointer++;
            if (!m_runs[step.mergeMinRun].hasMore())
                m_runs[step.mergeMinRun].exhausted = true;
        }

        // Find the merge-head block that will fly to output
        int flyVisIdx = -1;
        {
            int cnt = 0;
            for (int i = 0; i < m_runs.size(); ++i) {
                if (m_runs[i].exhausted && i != step.mergeMinRun) { continue; }
                if (!m_runs[i].hasMore() && i != step.mergeMinRun) { continue; }
                // count non-exhausted runs (before this step's pointer advance)
                if (i == step.mergeMinRun) { flyVisIdx = cnt; }
                cnt++;
            }
        }
        // Compute fly-from position (merge head in RAM area)
        QPointF flyFrom(m_layout.ramArea.center().x(),
                        m_layout.ramArea.center().y());
        if (flyVisIdx >= 0 && flyVisIdx < m_ramBlocks.size())
            flyFrom = m_ramBlocks[flyVisIdx]->pos();

        // Add output block(s) with animated fly-in
        for (double v : step.values) {
            m_totalOutputWritten++;
            m_allOutputValues.append(v);

            // Recalc output adaptive sizing
            m_layout.recalcOutput(m_totalOutputWritten);

            if (m_layout.densityMode) {
                // In density mode we don't create BlockItems — drawn in background
                // Remove any remaining block items
                if (!m_outputBlocks.isEmpty()) clearOutputBlocks();
                invalidateBackground();
                continue;
            }

            // Sliding window: evict oldest if over capacity
            while (m_outputBlocks.size() >= m_layout.outputCapacity) {
                BlockItem* old = m_outputBlocks.takeFirst();
                m_scene->removeItem(old);
                delete old;
            }

            // Reposition existing output blocks for sliding window
            for (int oi = 0; oi < m_outputBlocks.size(); ++oi) {
                QPointF target = m_layout.outputBlockPos(oi);
                m_outputBlocks[oi]->setBlockSize(m_layout.outputBlockW,
                                                  m_layout.outputBlockH);
                m_outputBlocks[oi]->setSnappedPos(target);
            }

            // Create new output block at fly-from position, animate to slot
            int slotIdx = m_outputBlocks.size();
            QPointF endPos = m_layout.outputBlockPos(slotIdx);
            BlockItem* b = createBlock(v, flyFrom);
            b->setBlockSize(m_layout.outputBlockW, m_layout.outputBlockH);
            b->setBlockOpacity(0.5);

            // Fly-in animation
            auto* posAnim = new QPropertyAnimation(b, "pos", b);
            posAnim->setDuration(280);
            posAnim->setStartValue(flyFrom);
            posAnim->setEndValue(endPos);
            posAnim->setEasingCurve(QEasingCurve::InOutCubic);
            posAnim->start(QAbstractAnimation::DeleteWhenStopped);

            // Opacity fade in
            auto* opAnim = new QPropertyAnimation(b, "blockOpacity", b);
            opAnim->setDuration(200);
            opAnim->setStartValue(0.5);
            opAnim->setEndValue(1.0);
            opAnim->setEasingCurve(QEasingCurve::OutCubic);
            opAnim->start(QAbstractAnimation::DeleteWhenStopped);

            m_outputBlocks.append(b);
        }

        // Refresh merge heads in RAM area
        clearRamBlocks();
        int visRunCount = std::min(static_cast<int>(m_runs.size()),
                                   MAX_VIS_BLOCKS);
        for (int i = 0; i < visRunCount; ++i) {
            if (!m_runs[i].hasMore()) continue;
            QPointF pos = m_layout.ramBlockPos(m_ramBlocks.size());
            BlockItem* b = createBlock(m_runs[i].headValue(), pos);
            b->setBlockOpacity(1.0);
            b->setBlockScale(1.0);
            m_ramBlocks.append(b);
        }
        invalidateBackground();   // run pills + output counter changed
        break;
    }

    default: break;
    }

    m_selectedMinRun = -1;
}

// ====================================================================
//  STEP EXECUTION — per-frame visual updates
//  These only modify BlockItem properties → per-item repaint (no full redraw)
// ====================================================================

void ExternalSortCanvas::executeLoadChunk(const AggregatedStep& /*step*/,
                                           double progress)
{
    // Fade glow as blocks settle
    for (auto* b : m_ramBlocks)
        b->setGlowIntensity(std::max(0.0, 0.6 * (1.0 - progress)));
}

void ExternalSortCanvas::executeSortInRAM(const AggregatedStep& /*step*/,
                                           double progress)
{
    for (int i = 0; i < m_ramBlocks.size(); ++i) {
        double phase = progress * M_PI * 3 + i * 0.3;
        m_ramBlocks[i]->setGlowIntensity(0.3 + 0.3 * std::sin(phase));
    }
}

void ExternalSortCanvas::executeWriteRun(const AggregatedStep& /*step*/,
                                          double progress)
{
    double e = progress < 0.5
                   ? 4 * progress * progress * progress
                   : 1 - std::pow(-2 * progress + 2, 3) / 2;
    for (auto* b : m_ramBlocks) {
        b->setBlockScale(1.0 - 0.4 * e);
        b->setBlockOpacity(1.0 - e);
        b->setGlowIntensity((1.0 - e) * 0.5);
        b->setGlowColor(QColor(0, 200, 100));
    }
}

void ExternalSortCanvas::executeMergeStep(const AggregatedStep& step,
                                           double progress)
{
    if (progress < 0.3) {
        // Phase 1 — all heads glow
        for (auto* b : m_ramBlocks) {
            b->setGlowIntensity(0.5);
            b->setGlowColor(QColor(0, 180, 220));
        }
    } else if (progress < 0.6) {
        // Phase 2 — highlight minimum
        int visIdx = -1;
        {
            int cnt = 0;
            for (int i = 0; i < m_runs.size(); ++i) {
                if (!m_runs[i].hasMore()) continue;
                if (i == step.mergeMinRun) { visIdx = cnt; break; }
                cnt++;
            }
        }
        for (int i = 0; i < m_ramBlocks.size(); ++i) {
            if (i == visIdx) {
                m_ramBlocks[i]->setGlowIntensity(1.0);
                m_ramBlocks[i]->setGlowColor(QColor(255, 200, 0));
                m_ramBlocks[i]->setBlockScale(1.15);
            } else {
                m_ramBlocks[i]->setGlowIntensity(0.15);
                m_ramBlocks[i]->setBlockScale(1.0);
            }
        }
    } else {
        // Phase 3 — min block flies toward output
        int visIdx = -1;
        {
            int cnt = 0;
            for (int i = 0; i < m_runs.size(); ++i) {
                if (!m_runs[i].hasMore()) continue;
                if (i == step.mergeMinRun) { visIdx = cnt; break; }
                cnt++;
            }
        }
        if (visIdx >= 0 && visIdx < m_ramBlocks.size()) {
            double subP = (progress - 0.6) / 0.4;
            double e = subP < 0.5
                           ? 4 * subP * subP * subP
                           : 1 - std::pow(-2 * subP + 2, 3) / 2;
            QPointF start = m_layout.ramBlockPos(visIdx);
            QPointF end(m_layout.outputArea.center().x(),
                        m_layout.outputArea.top() + 40);
            QPointF interp = start + (end - start) * e;
            m_ramBlocks[visIdx]->setSnappedPos(interp);
            m_ramBlocks[visIdx]->setBlockOpacity(1.0 - e * 0.5);
        }
    }
}

void ExternalSortCanvas::executeWriteOutput(const AggregatedStep&, double) {}
void ExternalSortCanvas::executePhaseTransition(const AggregatedStep&, double) {}

// ====================================================================
//  PAINTING — cached drawBackground + retained BlockItems + foreground
// ====================================================================

static void drawRoundedRectHelper(QPainter* p, const QRectF& r, qreal radius,
                                  const QPen& pen, const QBrush& brush)
{
    p->setPen(pen);
    p->setBrush(brush);
    p->drawRoundedRect(r.adjusted(1, 1, -1, -1), radius, radius);
}

static void drawSectionLabel(QPainter* p, const QString& text,
                             const QRectF& area, const QColor& color)
{
    p->save();
    QFont f;
    f.setPixelSize(13);
    f.setBold(true);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 1.2);
    p->setFont(f);
    p->setPen(color);
    p->drawText(QPointF(area.left() + 14, area.top() + 18), text);
    p->restore();
}

static void drawProgressBar(QPainter* p, const QRectF& r,
                            double progress, const QColor& color)
{
    p->save();
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(30, 32, 38));
    p->drawRoundedRect(r, 3, 3);

    QRectF fill(r.left(), r.top(),
                r.width() * std::clamp(progress, 0.0, 1.0), r.height());
    QLinearGradient g(fill.topLeft(), fill.topRight());
    g.setColorAt(0, color);
    g.setColorAt(1, color.lighter(120));
    p->setBrush(g);
    p->drawRoundedRect(fill, 3, 3);
    p->restore();
}

void ExternalSortCanvas::drawBackground(QPainter* painter, const QRectF& /*rect*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    QRectF sceneR = m_scene->sceneRect();

    // ---- Full background gradient ----
    QLinearGradient bg(0, 0, 0, sceneR.height());
    bg.setColorAt(0.0, QColor(22, 24, 28));
    bg.setColorAt(0.5, QColor(18, 20, 24));
    bg.setColorAt(1.0, QColor(14, 16, 20));
    painter->fillRect(sceneR, bg);

    // ---- DISK section ----
    {
        const QRectF& a = m_layout.diskArea;
        drawRoundedRectHelper(painter, a, 8,
                              QPen(QColor(60, 65, 75), 1.5),
                              QBrush(QColor(35, 38, 44, 200)));
        drawSectionLabel(painter, "DISK STORAGE", a, QColor(140, 150, 165));

        painter->setPen(QColor(100, 110, 125));
        QFont f; f.setPixelSize(11); painter->setFont(f);
        QString info = QString("%1 elements").arg(m_totalElements);
        if (m_diskLoaded > 0 && m_diskLoaded < m_totalElements)
            info += QString("  |  Loaded: %1").arg(m_diskLoaded);
        painter->drawText(QPointF(a.left() + 14, a.bottom() - 8), info);

        if (m_totalElements > 0 && m_diskLoaded > 0) {
            double prog = double(m_diskLoaded) / m_totalElements;
            QRectF bar(a.left() + 14, a.top() + 28, a.width() - 28, 6);
            drawProgressBar(painter, bar, prog, QColor(0, 140, 180));
        }
    }

    // ---- RAM section ----
    {
        const QRectF& a = m_layout.ramArea;
        QLinearGradient ramBg(a.topLeft(), a.bottomLeft());
        ramBg.setColorAt(0, QColor(38, 48, 62, 220));
        ramBg.setColorAt(1, QColor(28, 38, 52, 220));
        drawRoundedRectHelper(painter, a, 8,
                              QPen(QColor(0, 150, 180), 2), QBrush(ramBg));

        QString label = m_inMergePhase ? "MERGE HEADS" : "RAM BUFFER";
        drawSectionLabel(painter, label, a, QColor(0, 200, 220));

        // Status text
        if (!m_currentStep.statusText.isEmpty() &&
            (m_currentStep.type == AggregatedStep::Type::LoadChunk ||
             m_currentStep.type == AggregatedStep::Type::SortInRAM ||
             m_currentStep.type == AggregatedStep::Type::WriteRun  ||
             m_currentStep.type == AggregatedStep::Type::MergeStep)) {
            painter->save();
            painter->setPen(QColor(150, 160, 175, 180));
            QFont sf; sf.setPixelSize(12); sf.setItalic(true); painter->setFont(sf);
            QRectF textR(a.right() - 320, a.top() + 4, 310, 22);
            painter->drawText(textR, Qt::AlignRight | Qt::AlignVCenter,
                              m_currentStep.statusText);
            painter->restore();
        }
    }

    // ---- RUNS section ----
    {
        const QRectF& a = m_layout.runsArea;
        drawRoundedRectHelper(painter, a, 8,
                              QPen(QColor(90, 70, 110), 1.5),
                              QBrush(QColor(40, 35, 48, 200)));
        drawSectionLabel(painter,
                         QString("SORTED RUNS (%1)").arg(m_runsGenerated),
                         a, QColor(160, 130, 200));

        if (!m_runs.isEmpty()) {
            qreal pillH = std::min(26.0,
                (a.height() - 40) / std::max(1, (int)m_runs.size() / 8 + 1));
            qreal pillW = std::min(150.0,
                (a.width() - 20) / std::min((int)m_runs.size(), 8) - 8);
            int cols = std::max(1,
                static_cast<int>((a.width() - 20) / (pillW + 8)));

            for (int i = 0; i < m_runs.size(); ++i) {
                int row = i / cols, col = i % cols;
                QRectF pill(a.left() + 10 + col * (pillW + 8),
                            a.top() + 30 + row * (pillH + 5),
                            pillW, pillH);
                if (pill.bottom() > a.bottom() - 4) break;

                const SortRun& run = m_runs[i];
                painter->save();
                QColor pbg, border, txt;
                if (run.exhausted) {
                    pbg = QColor(50,50,55);  border = QColor(70,70,75);
                    txt = QColor(100,100,105);
                } else if (run.highlighted) {
                    pbg = QColor(80,60,20);  border = QColor(255,200,0);
                    txt = QColor(255,220,100);
                } else if (run.active) {
                    pbg = QColor(40,50,70);  border = QColor(0,150,200);
                    txt = QColor(0,200,230);
                } else {
                    pbg = QColor(55,45,65);  border = QColor(100,80,130);
                    txt = QColor(180,160,210);
                }
                painter->setPen(QPen(border, run.highlighted ? 2.0 : 1.0));
                painter->setBrush(pbg);
                painter->drawRoundedRect(pill, 4, 4);

                QFont pf;
                pf.setPixelSize(std::max(9,
                    static_cast<int>(pill.height() * 0.5)));
                pf.setBold(run.highlighted);
                painter->setFont(pf);
                painter->setPen(txt);

                QString plabel = QString("R%1 [%2]")
                                     .arg(run.index).arg(run.totalSize);
                if (run.hasMore() && m_inMergePhase)
                    plabel += QString::fromUtf8(" \xe2\x86\x92 %1")
                                  .arg(run.headValue(), 0, 'f', 0);
                painter->drawText(pill, Qt::AlignCenter, plabel);
                painter->restore();
            }
        }
    }

    // ---- OUTPUT section ----
    {
        const QRectF& a = m_layout.outputArea;
        drawRoundedRectHelper(painter, a, 8,
                              QPen(QColor(70, 130, 70), 1.5),
                              QBrush(QColor(30, 45, 30, 200)));
        QString olabel = QString("OUTPUT (%1 / %2)")
                             .arg(m_totalOutputWritten).arg(m_totalElements);
        drawSectionLabel(painter, olabel, a, QColor(100, 200, 100));

        if (m_totalElements > 0 && m_totalOutputWritten > 0) {
            double prog = double(m_totalOutputWritten) / m_totalElements;
            QRectF bar(a.left() + 14, a.top() + 28, a.width() - 28, 8);
            drawProgressBar(painter, bar, prog, QColor(80, 180, 80));
        }

        // Density mode: render bars instead of BlockItems
        if (m_layout.densityMode && !m_allOutputValues.isEmpty()) {
            drawDensityBars(painter, a);
        }
    }
}

void ExternalSortCanvas::drawForeground(QPainter* painter, const QRectF& /*rect*/)
{
    if (m_overlayOpacity < 0.01) return;

    painter->save();
    painter->resetTransform();
    painter->setOpacity(m_overlayOpacity);
    painter->fillRect(viewport()->rect(), QColor(0, 0, 0, 160));

    QFont f; f.setPixelSize(38); f.setBold(true); painter->setFont(f);
    painter->setPen(QColor(0, 220, 240));
    painter->drawText(viewport()->rect(), Qt::AlignCenter, m_overlayText);
    painter->restore();
}

// ====================================================================
//  EVENTS
// ====================================================================

void ExternalSortCanvas::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    // Scene rect = viewport rect (no scrolling)
    QRectF sceneR(0, 0, viewport()->width(), viewport()->height());
    m_scene->setSceneRect(sceneR);
    setSceneRect(sceneR);

    m_layout.invalidate();
    recalculateLayout();
    invalidateBackground();
}

void ExternalSortCanvas::wheelEvent(QWheelEvent* e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        e->angleDelta().y() > 0 ? zoomIn() : zoomOut();
    }
    e->accept();
}

void ExternalSortCanvas::mousePressEvent(QMouseEvent* e)
{
    QPointF pos = mapToScene(e->position().toPoint());
    if      (m_layout.diskArea.contains(pos))   emit regionClicked("disk");
    else if (m_layout.ramArea.contains(pos))    emit regionClicked("ram");
    else if (m_layout.runsArea.contains(pos))   emit regionClicked("runs");
    else if (m_layout.outputArea.contains(pos)) emit regionClicked("output");
}

// ====================================================================
//  HELPERS
// ====================================================================

void ExternalSortCanvas::recalculateLayout()
{
    int visCount = std::max(1, static_cast<int>(m_ramBlocks.size()));
    if (visCount <= 1) visCount = 16;
    m_layout.calculate(size(), visCount);

    // Recalc output adaptive sizing
    m_layout.recalcOutput(m_totalOutputWritten);

    // Update existing RAM block sizes and positions
    for (int i = 0; i < m_ramBlocks.size(); ++i) {
        m_ramBlocks[i]->setBlockSize(m_layout.blockWidth, m_layout.blockHeight);
        m_ramBlocks[i]->setSnappedPos(m_layout.ramBlockPos(i));
    }

    // Rebuild output window with new sizing
    rebuildOutputWindow();
}

void ExternalSortCanvas::rebuildOutputWindow()
{
    if (m_layout.densityMode) {
        // In density mode, remove all BlockItems — drawing handled in background
        if (!m_outputBlocks.isEmpty()) clearOutputBlocks();
        return;
    }

    // Determine sliding window range
    int cap   = m_layout.outputCapacity;
    int total = m_allOutputValues.size();
    int start = std::max(0, total - cap);
    int count = total - start;

    // If the window matches current blocks, just reposition
    if (m_outputBlocks.size() == count) {
        for (int i = 0; i < count; ++i) {
            m_outputBlocks[i]->setBlockSize(m_layout.outputBlockW,
                                             m_layout.outputBlockH);
            m_outputBlocks[i]->setSnappedPos(m_layout.outputBlockPos(i));
        }
        return;
    }

    // Full rebuild required
    clearOutputBlocks();
    for (int i = 0; i < count; ++i) {
        double v = m_allOutputValues[start + i];
        QPointF pos = m_layout.outputBlockPos(i);
        BlockItem* b = createBlock(v, pos);
        b->setBlockSize(m_layout.outputBlockW, m_layout.outputBlockH);
        b->setBlockOpacity(1.0);
        m_outputBlocks.append(b);
    }
}

void ExternalSortCanvas::drawDensityBars(QPainter* painter, const QRectF& area)
{
    // Compact density/timeline bars drawn in the background (no BlockItems)
    qreal left   = area.left() + 14;
    qreal right  = area.right() - 14;
    qreal availW = right - left;
    qreal top    = area.top() + 42;
    qreal bot    = area.bottom() - 8;
    qreal availH = bot - top;
    if (availW <= 0 || availH <= 0) return;

    int total = m_allOutputValues.size();
    if (total <= 0) return;

    // Number of bars = min(total, available pixel columns)
    int barCount = std::min(total, static_cast<int>(availW));
    qreal barW   = availW / barCount;

    painter->save();
    painter->setPen(Qt::NoPen);

    double stride = double(total) / barCount;
    for (int i = 0; i < barCount; ++i) {
        int idx = static_cast<int>(i * stride);
        if (idx >= total) idx = total - 1;
        double v = m_allOutputValues[idx];

        // Normalize value to [0,1]
        double norm = (m_dataMax > m_dataMin)
                          ? (v - m_dataMin) / (m_dataMax - m_dataMin) : 0.5;
        norm = std::clamp(norm, 0.0, 1.0);

        // Bar height proportional to value
        qreal bh = std::max(2.0, norm * availH);
        QRectF barRect(left + i * barW, bot - bh, std::max(barW - 0.5, 1.0), bh);

        QColor c = QColor::fromHsvF(0.33 - norm * 0.33, 0.7, 0.75 + norm * 0.2);
        painter->setBrush(c);
        painter->drawRect(barRect);
    }
    painter->restore();
}

QColor ExternalSortCanvas::valueToColor(double value) const
{
    double norm = (m_dataMax > m_dataMin)
                      ? (value - m_dataMin) / (m_dataMax - m_dataMin) : 0.5;
    norm = std::clamp(norm, 0.0, 1.0);
    return QColor::fromHsvF(0.6 - norm * 0.4, 0.65, 0.82);
}

QVector<double> ExternalSortCanvas::sampleValues(const QVector<double>& vals,
                                                  int maxCount) const
{
    if (vals.size() <= maxCount) return vals;
    QVector<double> out;
    double stride = double(vals.size()) / maxCount;
    for (int i = 0; i < maxCount; ++i)
        out.append(vals[static_cast<int>(i * stride)]);
    return out;
}
