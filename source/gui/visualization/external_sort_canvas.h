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

// ============================================================================
// SORT RUN — Abstract representation of a sorted run stored on disk.
// Only the head value (merge pointer) is actually rendered.
// ============================================================================
struct SortRun {
    int  index      = 0;
    int  totalSize  = 0;
    QVector<double> values;       // sorted values (sampled for display)
    int  readPointer = 0;         // current merge read position
    bool active      = false;     // participating in current merge
    bool highlighted = false;     // is the current minimum candidate
    bool exhausted   = false;     // all elements consumed

    double headValue() const {
        return (readPointer < values.size()) ? values[readPointer] : 0.0;
    }
    bool hasMore() const { return readPointer < values.size(); }
};

// ============================================================================
// AGGREGATED ANIMATION STEP
// ============================================================================
struct AggregatedStep {
    enum class Type {
        None,
        IntroPhase,
        LoadChunk,          // Blocks load from disk -> RAM
        SortInRAM,          // In-place sort visualization
        WriteRun,           // Write sorted chunk -> Run
        MergePhase,         // Phase transition to merge
        MergeStep,          // K-way merge: select min, write output
        WriteOutput,        // Write to final output
        PhaseTransition,    // Visual phase label
        Complete
    };

    Type type = Type::None;

    QVector<int>    sourceBlocks;   // block / run indices
    QVector<int>    targetBlocks;
    QVector<double> values;         // actual data values carried

    double  duration       = 300.0;
    QString statusText;
    QPointF focusPoint;
    int     operationCount = 1;

    // Merge-specific
    int     runIndex       = -1;    // which run this relates to
    int     mergeMinRun    = -1;    // which run had the minimum

    bool isComplete() const { return type == Type::Complete; }
};

// ============================================================================
// ANIMATION ENGINE — Centralized 60 fps fixed-timestep controller
// ============================================================================
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

// ============================================================================
// BLOCK ITEM — QGraphicsObject for a single data block.
// Uses Q_PROPERTY for QPropertyAnimation-driven movement.
// Paint is per-item: only dirty blocks are repainted (retained rendering).
// ============================================================================
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

    // Animated properties
    qreal blockOpacity()  const { return m_opacity; }
    void  setBlockOpacity(qreal o);

    qreal blockScale() const { return m_scale; }
    void  setBlockScale(qreal s);

    qreal glowIntensity() const { return m_glow; }
    void  setGlowIntensity(qreal g);

    // Data
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

// ============================================================================
// CANVAS LAYOUT — Viewport-aware region computation.
// OUTPUT section gets a guaranteed minimum height (OUTPUT_MIN_H).
// ============================================================================
struct CanvasLayout {
    QRectF diskArea;
    QRectF ramArea;
    QRectF runsArea;
    QRectF outputArea;

    // RAM block sizing
    qreal blockWidth  = 60;
    qreal blockHeight = 78;
    qreal spacing     = 10;
    qreal margin      = 6;
    qreal sectionGap  = 14;
    int   maxBlocksPerRow = 16;

    // OUTPUT adaptive block sizing
    qreal outputBlockW   = 60;   // adaptive width for output blocks
    qreal outputBlockH   = 78;   // adaptive height for output blocks
    int   outputCapacity = 20;   // max visible output blocks (sliding window)
    bool  densityMode    = false; // true when outputBlockW < 12 px

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

// ============================================================================
// EXTERNAL SORT CANVAS — QGraphicsView retained-rendering visualizer.
//
// DESIGN:
//   - QGraphicsView + QGraphicsScene for hardware-accelerated dirty-rect updates
//   - drawBackground()  renders cached section backgrounds / labels / run pills
//   - BlockItem scene objects for animated data blocks (per-item repaint only)
//   - drawForeground()  renders the phase overlay
//   - Integer pixel snapping on all block positions (anti-blur)
//   - QPropertyAnimation for block movement, opacity, scale
//   - Guaranteed OUTPUT area minimum height (180 px)
// ============================================================================
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
    // ---- Scene management ----
    BlockItem* createBlock(double value, const QPointF& pos);
    void       clearRamBlocks();
    void       clearOutputBlocks();

    // ---- Step execution (visual-only, per frame) ----
    void executeLoadChunk(const AggregatedStep& step, double progress);
    void executeSortInRAM(const AggregatedStep& step, double progress);
    void executeWriteRun(const AggregatedStep& step, double progress);
    void executeMergeStep(const AggregatedStep& step, double progress);
    void executeWriteOutput(const AggregatedStep& step, double progress);
    void executePhaseTransition(const AggregatedStep& step, double progress);

    // ---- Helpers ----
    void   recalculateLayout();
    void   rebuildOutputWindow();
    void   drawDensityBars(QPainter* painter, const QRectF& area);
    QColor valueToColor(double value) const;
    QVector<double> sampleValues(const QVector<double>& vals, int max) const;
    void   invalidateBackground();

    // ---- Scene ----
    QGraphicsScene* m_scene = nullptr;

    // ---- Engine ----
    AnimationEngine* m_engine = nullptr;

    // ---- VIEWPORT STATE (never one-block-per-element) ----
    int    m_totalElements = 0;
    double m_dataMin = 0.0, m_dataMax = 1.0;
    int    m_diskLoaded    = 0;

    QVector<BlockItem*>    m_ramBlocks;       // current RAM buffer blocks
    QVector<SortRun>       m_runs;            // sorted runs on disk
    QVector<BlockItem*>    m_outputBlocks;    // visible output blocks (sliding window)
    QVector<double>        m_allOutputValues; // ALL output values (for density mode)
    int    m_totalOutputWritten = 0;

    // Merge state
    bool   m_inMergePhase   = false;
    int    m_selectedMinRun  = -1;

    // Canvas state
    CanvasLayout m_layout;
    qreal m_zoom = 1.0;

    // Overlay
    QString m_overlayText;
    qreal   m_overlayOpacity = 0.0;
    QPropertyAnimation* m_overlayAnim = nullptr;

    // Render state
    AggregatedStep m_currentStep;
    double m_stepProgress  = 0.0;
    int    m_runsGenerated = 0;

    static constexpr int MAX_VIS_BLOCKS = 64;
};

#endif // EXTERNAL_SORT_CANVAS_H
