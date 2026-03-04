#ifndef EXTERNAL_SORT_CANVAS_H
#define EXTERNAL_SORT_CANVAS_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QElapsedTimer>
#include <QVector>
#include <QPointF>
#include <QRectF>
#include <QTimer>
#include <QQueue>
#include <QPainter>
#include <QPainterPath>
#include <cmath>
#include <algorithm>

class AnimationEngine;
class BlockItem;

/**
 * @brief Abstract representation of a sorted run stored on disk.
 *
 * Only the head value (the current merge pointer) is actually rendered.
 */
struct SortRun {
    int             index       = 0;     ///< Run index (0-based).
    int             totalSize   = 0;     ///< Total number of elements in the run.
    QVector<double> values;              ///< Sampled sorted values for display.
    int             readPointer = 0;     ///< Current merge read position.
    bool            active      = false; ///< True if participating in the current merge.
    bool            highlighted = false; ///< True if this run holds the current minimum.
    bool            exhausted   = false; ///< True when all elements have been consumed.

    double headValue() const {
        return (readPointer < values.size()) ? values[readPointer] : 0.0;
    }
    bool hasMore() const { return readPointer < values.size(); }
};

/**
 * @brief A high-level visual animation step produced by StepAggregator.
 *
 * Multiple raw algorithm operations are batched into a single AggregatedStep
 * to reduce rendering overhead while preserving visual fidelity.
 */
struct AggregatedStep {
    /** @brief Visual operation types for aggregated animation steps. */
    enum class Type {
        None,
        IntroPhase,
        LoadChunk,       ///< Blocks loading from disk into RAM.
        SortInRAM,       ///< In-place sort visualization inside RAM.
        WriteRun,        ///< Writing sorted chunk to a run file.
        MergePhase,      ///< Phase transition to the merge stage.
        MergeStep,       ///< K-way merge: select minimum, write to output.
        WriteOutput,     ///< Write final merged element to output file.
        PhaseTransition, ///< Visual phase label transition.
        Complete         ///< Sorting is fully complete.
    };

    Type type = Type::None; ///< The type of visual operation.

    QVector<int>    sourceBlocks;  ///< Source block or run indices.
    QVector<int>    targetBlocks;  ///< Target block or run indices.
    QVector<double> values;        ///< Actual data values carried by this step.

    double  duration       = 300.0; ///< Recommended display duration in milliseconds.
    QString statusText;             ///< Status message to show in the UI.
    QPointF focusPoint;             ///< Camera focus point for this step.
    int     operationCount = 1;     ///< Number of raw operations aggregated into this step.

    int runIndex    = -1; ///< Run this step relates to (-1 = none).
    int mergeMinRun = -1; ///< Run holding the minimum during a MergeStep (-1 = none).

    /// @brief Returns true when this step signals sort completion.
    bool isComplete() const { return type == Type::Complete; }
};

/**
 * @brief Centralized 60 fps fixed-timestep animation engine.
 *
 * Drives step execution via a QTimer, interpolates progress between steps,
 * and emits frame-rate-aligned signals for the canvas to respond to.
 */
class AnimationEngine : public QObject
{
    Q_OBJECT
public:
    explicit AnimationEngine(QObject* parent = nullptr);
    ~AnimationEngine();

    void start();
    void stop();
    void pause();
    void resume();
    bool isRunning() const { return m_running; }
    bool isPaused()  const { return m_paused;  }

    void enqueueStep(const AggregatedStep& step);
    void enqueueSteps(const QVector<AggregatedStep>& steps);
    void clearQueue();
    int  queueSize()  const { return m_stepQueue.size(); }
    bool hasSteps()   const { return !m_stepQueue.isEmpty(); }

    void stepForward();
    void stepBackward();
    void seekToStep(int index);
    int  currentStepIndex() const { return m_currentStepIndex; }
    int  totalSteps()       const { return m_allSteps.size(); }

    void   setSpeedMultiplier(double m) { m_speed = std::clamp(m, 0.1, 10.0); }
    double speedMultiplier()      const { return m_speed; }

    static constexpr double TARGET_FPS    = 60.0;
    static constexpr double FRAME_TIME_MS = 1000.0 / TARGET_FPS;
    double currentProgress() const { return m_stepProgress; }
    double deltaTime()       const { return m_deltaTime; }

signals:
    void frameUpdate(double deltaTime, const AggregatedStep& step, double progress);
    void stepStarted(const AggregatedStep& step);
    void stepCompleted(const AggregatedStep& step);
    void stepIndexChanged(int current, int total);
    void queueSizeChanged(int size);
    void queueEmpty();
    void playbackStarted();
    void playbackStopped();

private slots:
    void onTimerTick();

private:
    void   advanceStep();
    double calculateDuration(const AggregatedStep& step) const;
    double adaptivePacing() const;

    QTimer*        m_timer = nullptr;
    QElapsedTimer  m_clock;

    QQueue<AggregatedStep>  m_stepQueue;
    QVector<AggregatedStep> m_allSteps;
    int            m_currentStepIndex = -1;
    AggregatedStep m_currentStep;

    double m_stepProgress = 0.0;
    double m_stepDuration = 300.0;
    double m_stepElapsed  = 0.0;
    double m_deltaTime    = 0.0;
    double m_accumulator  = 0.0;
    qint64 m_lastFrameTime = 0;

    bool   m_running = false;
    bool   m_paused  = false;
    double m_speed   = 1.0;
    int    m_executedSteps = 0;
};

/**
 * @brief QGraphicsObject representing a single data block in the canvas.
 *
 * Exposes animated Q_PROPERTYs (opacity, scale, glow) for QPropertyAnimation-
 * driven transitions. Only dirty items are repainted (retained rendering).
 */
class BlockItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(qreal blockOpacity  READ blockOpacity  WRITE setBlockOpacity)
    Q_PROPERTY(qreal blockScale    READ blockScale    WRITE setBlockScale)
    Q_PROPERTY(qreal glowIntensity READ glowIntensity WRITE setGlowIntensity)

public:
    explicit BlockItem(double value, const QColor& color,
                       qreal w, qreal h,
                       QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    /// @brief Animated properties
    qreal blockOpacity()  const { return m_opacity; }
    void  setBlockOpacity(qreal o);

    qreal blockScale() const { return m_scale; }
    void  setBlockScale(qreal s);

    qreal glowIntensity() const { return m_glow; }
    void  setGlowIntensity(qreal g);

    /// @brief Data
    double value() const { return m_value; }
    void   setValue(double v) { m_value = v; update(); }

    QColor color() const { return m_color; }
    void   setColor(const QColor& c) { m_color = c; update(); }

    QColor glowColor() const { return m_glowColor; }
    void   setGlowColor(const QColor& c) { m_glowColor = c; update(); }

    bool sorted() const { return m_sorted; }
    void setSorted(bool s) { m_sorted = s; update(); }

    qreal blockWidth()  const { return m_w; }
    qreal blockHeight() const { return m_h; }
    void  setBlockSize(qreal w, qreal h);

    // Integer-pixel-snapped position (prevents sub-pixel blur)
    void setSnappedPos(const QPointF& p) {
        setPos(std::round(p.x()), std::round(p.y()));
    }
    void setSnappedPos(qreal x, qreal y) {
        setPos(std::round(x), std::round(y));
    }

private:
    double m_value;
    QColor m_color;
    QColor m_glowColor{0, 200, 220};
    qreal  m_w, m_h;
    qreal  m_opacity = 1.0;
    qreal  m_scale   = 1.0;
    qreal  m_glow    = 0.0;
    bool   m_sorted  = false;
};

/**
 * @brief Viewport-aware region layout for the sort canvas.
 *
 * Divides the canvas into four named sections (Disk, RAM, Runs, Output)
 * and computes block sizes adaptively. The Output section is guaranteed
 * a minimum height of OUTPUT_MIN_H pixels.
 */
struct CanvasLayout {
    QRectF diskArea;   ///< Bounding rectangle for the disk section.
    QRectF ramArea;    ///< Bounding rectangle for the RAM section.
    QRectF runsArea;   ///< Bounding rectangle for the sorted runs section.
    QRectF outputArea; ///< Bounding rectangle for the output section.

    qreal blockWidth      = 60; ///< Width of each RAM block in pixels.
    qreal blockHeight     = 78; ///< Height of each RAM block in pixels.
    qreal spacing         = 10; ///< Spacing between RAM blocks.
    qreal margin          = 6;  ///< Inner margin around sections.
    qreal sectionGap      = 14; ///< Gap between adjacent sections.
    int   maxBlocksPerRow = 16; ///< Maximum blocks displayed per row in RAM.

    qreal outputBlockW   = 60;   ///< Adaptive width for output blocks.
    qreal outputBlockH   = 78;   ///< Adaptive height for output blocks.
    int   outputCapacity = 20;   ///< Maximum visible output blocks (sliding window).
    bool  densityMode    = false; ///< True when outputBlockW < DENSITY_THRESHOLD px.

    static constexpr qreal DISK_H   = 0.07;
    static constexpr qreal RAM_H    = 0.50;
    static constexpr qreal RUNS_H   = 0.21;
    static constexpr qreal OUTPUT_H = 0.22;
    static constexpr qreal OUTPUT_MIN_H = 180.0;
    static constexpr qreal OUT_BLOCK_MIN = 8.0;
    static constexpr qreal OUT_BLOCK_MAX = 70.0;
    static constexpr qreal DENSITY_THRESHOLD = 12.0;

    QSize cachedSize;
    int   cachedVisCount  = 0;
    int   cachedOutCount  = 0;
    bool  isValid = false;

    void    calculate(const QSize& size, int visibleBlockCount);
    void    recalcOutput(int outputCount);
    void    invalidate() { isValid = false; }
    void    calculateBlockSize(qreal width, qreal height, int visCount);
    QPointF ramBlockPos(int index) const;
    QPointF outputBlockPos(int index) const;
};

/**
 * @brief QGraphicsView-based retained-rendering sort visualizer.
 *
 * Design highlights:
 * - QGraphicsView + QGraphicsScene for dirty-rect GPU-accelerated updates.
 * - drawBackground() paints cached section backgrounds, labels, and run pills.
 * - BlockItem objects handle per-item repaint (only dirty blocks redraw).
 * - drawForeground() paints the phase-transition overlay.
 * - Integer pixel snapping on all block positions prevents sub-pixel blur.
 * - QPropertyAnimation drives block movement, opacity, and scale.
 * - Output section is guaranteed a minimum height of 180 px.
 */
class ExternalSortCanvas : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(qreal overlayOpacity READ overlayOpacity WRITE setOverlayOpacity)

public:
    explicit ExternalSortCanvas(QWidget* parent = nullptr);
    ~ExternalSortCanvas();

    // Data — stores metadata only, never individual blocks
    void setData(const QVector<double>& data);
    void clear();

    // Engine
    void             setEngine(AnimationEngine* engine);
    AnimationEngine* engine() const { return m_engine; }

    // Zoom
    void  zoomIn();
    void  zoomOut();
    void  resetZoom();

    // Phase overlay
    void  showPhaseOverlay(const QString& text, int ms = 1500);
    void  hidePhaseOverlay();
    qreal overlayOpacity() const { return m_overlayOpacity; }
    void  setOverlayOpacity(qreal o);

    // Info accessors (API compatibility)
    int                blockCount() const { return m_ramBlocks.size(); }
    const CanvasLayout& layout()   const { return m_layout; }

public slots:
    void onFrameUpdate(double dt, const AggregatedStep& step, double progress);
    void onStepStarted(const AggregatedStep& step);
    void onStepCompleted(const AggregatedStep& step);

signals:
    void visualizationReady();
    void regionClicked(const QString& region);
    void zoomChanged(qreal z);

protected:
    void resizeEvent(QResizeEvent*) override;
    void wheelEvent(QWheelEvent*)   override;
    void mousePressEvent(QMouseEvent*) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

private:
    /// @name Scene Management
    /// @{
    BlockItem* createBlock(double value, const QPointF& pos);
    void       clearRamBlocks();
    void       clearOutputBlocks();
    /// @}

    /// @name Step Execution (Visual-only, per frame)
    /// @{
    void executeLoadChunk(const AggregatedStep& step, double progress);
    void executeSortInRAM(const AggregatedStep& step, double progress);
    void executeWriteRun(const AggregatedStep& step, double progress);
    void executeMergeStep(const AggregatedStep& step, double progress);
    void executeWriteOutput(const AggregatedStep& step, double progress);
    void executePhaseTransition(const AggregatedStep& step, double progress);
    /// @}

    /// @name Helpers
    /// @{
    void   recalculateLayout();
    void   rebuildOutputWindow();
    void   drawDensityBars(QPainter* painter, const QRectF& area);
    QColor valueToColor(double value) const;
    QVector<double> sampleValues(const QVector<double>& vals, int max) const;
    void   invalidateBackground();
    /// @}

    QGraphicsScene*  m_scene  = nullptr; ///< Underlying graphics scene.
    AnimationEngine* m_engine = nullptr; ///< Animation engine providing frame updates.

    int    m_totalElements = 0;    ///< Total number of elements in the dataset.
    double m_dataMin = 0.0;        ///< Minimum value in the dataset (for color mapping).
    double m_dataMax = 1.0;        ///< Maximum value in the dataset (for color mapping).
    int    m_diskLoaded    = 0;    ///< Number of elements loaded from disk so far.

    QVector<BlockItem*> m_ramBlocks;       ///< Current RAM buffer block items.
    QVector<SortRun>    m_runs;            ///< Sorted runs currently on disk.
    QVector<BlockItem*> m_outputBlocks;    ///< Visible output block items (sliding window).
    QVector<double>     m_allOutputValues; ///< All output values written (for density mode).
    int    m_totalOutputWritten = 0;       ///< Total output elements written so far.

    bool m_inMergePhase  = false; ///< True once the merge phase begins.
    int  m_selectedMinRun = -1;   ///< Index of the run holding the merge minimum (-1 = none).

    CanvasLayout m_layout;     ///< Current canvas section layout.
    qreal        m_zoom = 1.0; ///< Current zoom factor.

    QString             m_overlayText;            ///< Phase overlay label text.
    qreal               m_overlayOpacity = 0.0;   ///< Phase overlay opacity (0–1).
    QPropertyAnimation* m_overlayAnim    = nullptr; ///< Overlay fade animation.

    AggregatedStep m_currentStep;        ///< Step being executed in the current frame.
    double         m_stepProgress  = 0.0; ///< Progress (0–1) within the current step.
    int            m_runsGenerated = 0;   ///< Number of sorted runs generated so far.

    static constexpr int MAX_VIS_BLOCKS = 64;
};

#endif // EXTERNAL_SORT_CANVAS_H
