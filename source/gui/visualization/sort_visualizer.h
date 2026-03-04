#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QElapsedTimer>
#include <QVector>
#include <QPointF>
#include <QScrollBar>

#include "../animation/animation_step.h"
#include "../animation/animation_controller.h"
#include "../animation/animation_clock.h"
#include "memory_block.h"
#include "moving_block.h"

/**
 * @brief Viewport-aware layout descriptor for the SortVisualizer widget.
 *
 * Divides the widget into six named regions and computes block sizes adaptively
 * based on the window size and visible block count.
 */
struct VisualizerLayout {
    QRectF heapArea;   ///< Top section: merge heap visualization.
    QRectF diskArea;   ///< Disk storage section.
    QRectF ramArea;    ///< RAM buffer (prominent center section).
    QRectF runsArea;   ///< Sorted runs section.
    QRectF outputArea; ///< Final output section.
    QRectF statusArea; ///< Compact status bar at the bottom.

    /// @name Block Dimensions
    /// @{
    qreal blockWidth  = 50;  ///< Default block width in pixels (enlarged for visibility).
    qreal blockHeight = 70;  ///< Default block height in pixels.
    qreal spacing     = 8;   ///< Spacing between adjacent blocks.
    qreal margin      = 15;  ///< Inner margin around each region.
    qreal sectionSpacing = 25; ///< Minimum spacing between sections.
    /// @}

    /// @name Adaptive Sizing
    /// @{
    qreal minBlockWidth    = 20;  ///< Minimum block width (lower adaptive bound).
    qreal maxBlockWidth    = 100; ///< Maximum block width (upper adaptive bound).
    int   visibleBlockCount = 0;  ///< Number of visible blocks in the current viewport.
    /// @}

    /** @brief Layout mode selected based on current window dimensions. */
    enum class LayoutMode {
        Compact,  ///< Small window layout.
        Standard, ///< Normal window layout.
        Expanded  ///< Large window or full-screen layout.
    };
    LayoutMode mode = LayoutMode::Standard; ///< Active layout mode.

    /// @name Cached Geometry
    /// @{
    bool  geometryCached = false; ///< True when the cached geometry is still valid.
    QSize lastWidgetSize;         ///< Widget size used during the last layout calculation.
    int   lastBlockCount = 0;     ///< Block count used during the last layout calculation.
    /// @}

    /** @brief Area height proportions as fractions of total available height. */
    struct Proportions {
        qreal heap   = 0.0;  ///< Merge heap area (only shown during merge phase).
        qreal disk   = 0.15; ///< Disk area (15%).
        qreal ram    = 0.35; ///< RAM area (35%, dominant).
        qreal runs   = 0.20; ///< Sorted runs area (20%).
        qreal output = 0.25; ///< Output area (25%).
        qreal status = 0.05; ///< Status bar (5%).
    } proportions; ///< Height proportion settings.

    /// @brief Recalculates the layout for @p widgetSize with @p blockCount blocks.
    void calculate(const QSize& widgetSize, int blockCount);
    /// @brief Recalculates the layout with zoom support.
    void calculateAdaptive(const QSize& widgetSize, int blockCount, qreal zoomFactor);

    /// @brief Forces a full layout recalculation on the next call to calculate().
    void invalidateCache() { geometryCached = false; }

    /// @brief Returns the optimal block width for @p availableWidth and @p blockCount.
    qreal calculateBlockWidth(qreal availableWidth, int blockCount) const;

    /// @brief Returns true if the layout should be recalculated for @p widgetSize / @p blockCount.
    bool needsUpdate(const QSize& widgetSize, int blockCount) const;
};

/**
 * @brief Virtualized rendering viewport tracking the visible block window.
 *
 * Enables smooth scrolling over large block arrays by tracking the scroll
 * offset and only rendering the blocks that fall within the visible range.
 */
struct Viewport {
    int   startIndex   = 0;   ///< Index of the first visible block.
    int   endIndex     = 0;   ///< Index of the last visible block.
    qreal scrollOffset = 0.0; ///< Current horizontal scroll offset in pixels.
    qreal totalWidth   = 0.0; ///< Total scrollable width in pixels.
    qreal visibleWidth = 0.0; ///< Width of the visible area in pixels.

    qreal targetOffset = 0.0;  ///< Target offset for smooth-scrolling interpolation.
    bool  isScrolling  = false; ///< True while a smooth-scroll animation is in progress.

    /// @brief Returns true if the block at @p index falls within the visible range.
    bool isBlockVisible(int index, qreal blockWidth, qreal spacing) const;
    /// @brief Converts a scene position to an adjusted viewport-relative position.
    QPointF adjustPosition(const QPointF& pos) const;
};

/**
 * @brief RGB color transition helper for smooth per-block color animations.
 *
 * Use transitionTo() to trigger a smooth cross-fade to a new color, then
 * call update() each frame with the elapsed delta time in milliseconds.
 */
struct ColorTransition {
    QColor startColor;              ///< Source color at the start of the transition.
    QColor endColor;                ///< Target color at the end of the transition.
    QColor currentColor;            ///< Interpolated color for the current frame.
    double progress = 1.0;          ///< Normalized transition progress (0–1; 1 = complete).
    double duration = 200.0;        ///< Transition duration in milliseconds.
    
    void transitionTo(const QColor& target, double durationMs = 200.0) {
        if (currentColor != target) {
            startColor = currentColor;
            endColor = target;
            duration = durationMs;
            progress = 0.0;
        }
    }
    
    void update(double deltaMs) {
        if (progress < 1.0) {
            progress = qMin(1.0, progress + deltaMs / duration);
            // Smooth interpolation
            double t = progress * progress * (3.0 - 2.0 * progress); // smoothstep
            currentColor = QColor::fromRgbF(
                startColor.redF() + (endColor.redF() - startColor.redF()) * t,
                startColor.greenF() + (endColor.greenF() - startColor.greenF()) * t,
                startColor.blueF() + (endColor.blueF() - startColor.blueF()) * t,
                startColor.alphaF() + (endColor.alphaF() - startColor.alphaF()) * t
            );
        }
    }
    
    bool isTransitioning() const { return progress < 1.0; }
};

/**
 * @brief Animated Bézier-curve arrow drawn between two canvas regions.
 *
 * Supports fade-in/fade-out opacity transitions and smooth progress
 * animation along a cubic Bézier path.
 */
struct AnimatedArrow {
    QPointF start;                      ///< Arrow tail position.
    QPointF end;                        ///< Arrow head position.
    QPointF control;                    ///< Bézier control point.
    double  progress      = 1.0;        ///< Normalized draw progress (0–1).
    double  opacity       = 1.0;        ///< Current opacity.
    double  targetOpacity = 1.0;        ///< Target opacity for fade transitions.
    QColor  color{0, 143, 150};         ///< Arrow color.
    bool    visible  = false;           ///< True when the arrow should be rendered.
    bool    useCurve = true;            ///< True to draw as a Bézier curve, false for straight line.
    
    /// @brief Returns the eased animation progress using a smoothstep curve.
    double easedProgress() const {
        // Smooth ease-in-out
        double t = progress;
        return t * t * (3.0 - 2.0 * t);
    }
    
    void render(QPainter* painter) const;
    void animate(double deltaTime, double speed = 1.0);
    
    /// @brief Fades the arrow in by setting the target opacity to 1.
    void fadeIn() { targetOpacity = 1.0; }
    /// @brief Fades the arrow out by setting the target opacity to 0.
    void fadeOut() { targetOpacity = 0.0; }
};

/**
 * @brief QPainter-based widget that visualizes external merge-sort in real time.
 *
 * Renders six canvas regions (disk, RAM, runs, output, heap, status) and drives
 * block animations via an AnimationController and a frame timer.
 */
class SortVisualizer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal cameraOffsetY READ cameraOffsetY WRITE setCameraOffsetY)
    Q_PROPERTY(qreal ramScale READ ramScale WRITE setRamScale)
    Q_PROPERTY(qreal introProgress READ introProgress WRITE setIntroProgress)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor NOTIFY zoomChanged)
    Q_PROPERTY(qreal viewportOffset READ viewportOffset WRITE setViewportOffset)
    Q_PROPERTY(qreal highlightIntensity READ highlightIntensity WRITE setHighlightIntensity)
    Q_PROPERTY(qreal transitionProgress READ transitionProgress WRITE setTransitionProgress)

public:
    explicit SortVisualizer(QWidget* parent = nullptr);
    ~SortVisualizer();
    
    /// @brief Attaches the animation controller that feeds steps into this widget.
    void setController(AnimationController* controller);
    /// @brief Returns the currently attached animation controller.
    AnimationController* getController() { return controller; }

    /// @brief Populates the visualizer with @p data values and resets all state.
    void setData(const QVector<double>& data);
    /// @brief Clears all visualized data and resets to the initial state.
    void clear();

    /// @brief Recalculates all region layout rectangles from the current widget size.
    void recalculateLayout();
    /// @brief Marks the layout as dirty so it is recalculated on the next paint.
    void invalidateLayout();
    
    /// @brief Camera Y offset animated property (used by QPropertyAnimation).
    qreal cameraOffsetY() const { return m_cameraOffsetY; }
    void setCameraOffsetY(qreal offset);

    qreal ramScale() const { return m_ramScale; }      ///< RAM region scale factor.
    void setRamScale(qreal scale);

    qreal introProgress() const { return m_introProgress; } ///< Introduction animation progress (0–1).
    void setIntroProgress(qreal progress);

    /// @brief Normalized highlight intensity animated property.
    qreal highlightIntensity() const { return m_highlightIntensity; }
    void setHighlightIntensity(qreal intensity);

    /// @brief General phase transition progress animated property (0–1).
    qreal transitionProgress() const { return m_transitionProgress; }
    void setTransitionProgress(qreal progress);

    /// @name Zoom and Viewport
    /// @{
    qreal zoomFactor() const { return m_zoomFactor; }
    void setZoomFactor(qreal zoom);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void setViewportOffset(qreal offset);
    qreal viewportOffset() const { return m_viewport.scrollOffset; }

    /// @brief Smoothly scrolls the viewport to @p offset over @p durationMs milliseconds.
    void smoothScrollTo(qreal offset, int durationMs = 300);

    /// @brief Scrolls the viewport just enough to make @p blockIndex visible.
    void ensureBlockVisible(int blockIndex);
    /// @brief Scrolls the viewport to center on @p blockIndex.
    void scrollToBlock(int blockIndex);
    /// @}

    /// @brief Returns the current disk block array (read-only).
    const QVector<MemoryBlock>& getDiskBlocks() const { return diskBlocks; }
    /// @brief Returns the current RAM block array (read-only).
    const QVector<MemoryBlock>& getRamBlocks() const { return ramBlocks; }

    /// @brief Exports all current block positions as an {id, position} list.
    QVector<QPair<int, QPointF>> exportBlockPositions() const;
    /// @brief Restores block positions from a previously exported snapshot.
    void importBlockPositions(const QVector<QPair<int, QPointF>>& positions);
    
public slots:
    /// @name Step Handling Slots
    /// @{
    void onStepStarted(const AnimationStep& step);
    void onStepProgress(const AnimationStep& step, double progress);
    void onStepCompleted(const AnimationStep& step);
    /// @}

    /// @name Frame Update Slots
    /// @{
    void onFrameUpdate(double deltaTime, double stepProgress);
    /// @}

    /// @name Playback State Slots
    /// @{
    void onPlaybackStarted();
    void onPlaybackStopped();
    /// @}

signals:
    void visualizationReady();
    void regionClicked(const QString& regionName);
    void zoomChanged(qreal zoomFactor);
    void viewportChanged(qreal offset, qreal visibleWidth, qreal totalWidth);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    /// @name Rendering Methods
    /// @{
    void drawBackground(QPainter& painter);
    void drawDiskArea(QPainter& painter);
    void drawRAMArea(QPainter& painter);
    void drawRunsArea(QPainter& painter);
    void drawOutputArea(QPainter& painter);
    void drawStatusArea(QPainter& painter);
    void drawArrows(QPainter& painter);
    void drawPhaseOverlay(QPainter& painter);
    void drawMovingBlocks(QPainter& painter);
    void drawViewportIndicator(QPainter& painter);
    /// @}

    /// @name Virtualized Rendering
    /// @{
    void drawVisibleBlocks(QPainter& painter, const QVector<MemoryBlock>& blocks,
                          const QRectF& area, const QString& areaName);
    bool shouldRenderBlock(int index) const;
    void updateViewport();
    /// @}

    /// @name Step Execution
    /// @{
    void executeIntroDisk(const AnimationStep& step, double progress);
    void executeHighlightRAM(const AnimationStep& step, double progress);
    void executeLoadToRAM(const AnimationStep& step, double progress);
    void executeSortBlock(const AnimationStep& step, double progress);
    void executeWriteRun(const AnimationStep& step, double progress);
    void executeMergeStep(const AnimationStep& step, double progress);
    void executeWriteOutput(const AnimationStep& step, double progress);
    void executeCameraFocus(const AnimationStep& step, double progress);
    void executeZoomRAM(const AnimationStep& step, double progress);
    void executePhaseTransition(const AnimationStep& step, double progress);
    /// @}

    /// @name Position Helpers
    /// @{
    QPointF getDiskBlockPosition(int index) const;
    QPointF getRAMBlockPosition(int index) const;
    QPointF getRunBlockPosition(int runIndex, int blockIndex) const;
    QPointF getOutputBlockPosition(int index) const;
    double  getMaxValue() const;
    void    updateBlockAnimations(double deltaTime);
    void    sortRAMBlocks();
    /// @}
    
    AnimationController* controller = nullptr; ///< Attached animation controller.

    VisualizerLayout layout; ///< Current region layout descriptor.

    QVector<MemoryBlock>              diskBlocks;   ///< Disk storage blocks.
    QVector<MemoryBlock>              ramBlocks;    ///< RAM buffer blocks.
    QVector<QVector<MemoryBlock>>     runBlocks;    ///< Sorted run blocks (outer = run index).
    QVector<MemoryBlock>              outputBlocks; ///< Final output blocks.

    MovingBlockGroup movingBlocks; ///< Flying/animated block overlays.

    QVector<AnimatedArrow> arrows; ///< Animated arrows connecting regions.

    AnimationStep currentStep;            ///< Animation step being executed this frame.
    double        currentProgress   = 0.0; ///< Normalized progress within the current step.
    bool          showPhaseOverlay  = false; ///< True when the phase-transition overlay is visible.
    QString       phaseText;               ///< Phase transition label text.
    double        phaseOverlayProgress = 0.0; ///< Opacity of the phase overlay (0–1).

    qreal m_cameraOffsetY = 0.0;  ///< Animated vertical camera offset.
    qreal m_ramScale      = 0.8;  ///< Animated RAM region scale factor.
    qreal m_introProgress = 0.0;  ///< Introduction animation progress (0–1).

    qreal m_highlightIntensity  = 0.0; ///< Animated highlight intensity (0–1).
    qreal m_transitionProgress  = 0.0; ///< Animated phase transition progress (0–1).
    QPropertyAnimation* m_highlightAnim  = nullptr; ///< Highlight intensity animation.
    QPropertyAnimation* m_transitionAnim = nullptr; ///< Transition progress animation.

    ColorTransition              m_diskHighlightColor;    ///< Smooth color transition for disk region.
    ColorTransition              m_ramHighlightColor;     ///< Smooth color transition for RAM region.
    QVector<ColorTransition>     m_blockHighlightColors;  ///< Per-block color transitions.

    qreal               m_zoomFactor = 1.0;      ///< Current zoom factor.
    qreal               m_minZoom    = 0.25;     ///< Minimum allowed zoom.
    qreal               m_maxZoom    = 4.0;      ///< Maximum allowed zoom.
    qreal               m_zoomStep   = 0.25;     ///< Zoom step per mouse-wheel tick.
    Viewport            m_viewport;              ///< Virtualized rendering viewport.
    QPropertyAnimation* m_zoomAnim   = nullptr;  ///< Zoom factor animation.
    QPropertyAnimation* m_scrollAnim = nullptr;  ///< Viewport scroll animation.

    bool diskHighlighted       = false; ///< True when the disk region is highlighted.
    bool ramHighlighted        = false; ///< True when the RAM region is highlighted.
    int  highlightedDiskBlock  = -1;    ///< Index of the highlighted disk block (-1 = none).
    int  highlightedRAMBlock   = -1;    ///< Index of the highlighted RAM block (-1 = none).
    QVector<int> comparedIndices;       ///< Indices of blocks being compared this step.

    QPropertyAnimation* cameraAnim   = nullptr; ///< Camera offset animation.
    QPropertyAnimation* ramScaleAnim = nullptr; ///< RAM scale animation.
    QPropertyAnimation* introAnim    = nullptr; ///< Introduction sequence animation.

    int m_defaultAnimDuration = 250; ///< Default transition duration (ms).
    int m_fastAnimDuration    = 150; ///< Fast transition duration (ms).
    int m_slowAnimDuration    = 400; ///< Slow transition duration (ms).

    QElapsedTimer frameTimer;           ///< High-precision frame timer.
    double        lastFrameTime  = 0.0; ///< Timestamp of the previous frame (ms).
    bool          m_pendingUpdate = false; ///< True when a repaint is already queued.

    QVector<QRectF> m_cachedDiskPositions; ///< Cached per-block disk region rects.
    QVector<QRectF> m_cachedRamPositions;  ///< Cached per-block RAM region rects.
    bool            m_geometryDirty = true; ///< True when cached block geometry is stale.

    int totalBlocks    = 0; ///< Total number of elements being sorted.
    int runsGenerated  = 0; ///< Number of sorted runs created so far.
    int mergeProgress  = 0; ///< Merge phase progress counter.
};
