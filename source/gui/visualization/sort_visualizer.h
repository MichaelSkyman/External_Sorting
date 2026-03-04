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

// Region definitions for layout - IMPROVED with better spacing
struct VisualizerLayout {
    QRectF heapArea;      // Top: Merge heap visualization
    QRectF diskArea;      // Disk storage
    QRectF ramArea;       // RAM buffer (prominent)
    QRectF runsArea;      // Sorted runs
    QRectF outputArea;    // Final output
    QRectF statusArea;    // Compact status bar
    
    // Block dimensions - ENLARGED for visibility
    qreal blockWidth = 50;
    qreal blockHeight = 70;
    qreal spacing = 8;           // Increased spacing
    qreal margin = 15;
    qreal sectionSpacing = 25;   // Minimum spacing between sections
    
    // Adaptive sizing
    qreal minBlockWidth = 20;    // Larger minimum
    qreal maxBlockWidth = 100;   // Larger maximum
    int visibleBlockCount = 0;
    
    // Layout mode for different window sizes
    enum class LayoutMode {
        Compact,    // Small window
        Standard,   // Normal window
        Expanded    // Large window/full screen
    };
    LayoutMode mode = LayoutMode::Standard;
    
    // Cached geometry to prevent recalculation jitter
    bool geometryCached = false;
    QSize lastWidgetSize;
    int lastBlockCount = 0;
    
    // Area proportions (percentages of available height)
    struct Proportions {
        qreal heap = 0.0;       // Only shown during merge
        qreal disk = 0.15;      // 15% for disk
        qreal ram = 0.35;       // 35% for RAM (dominant)
        qreal runs = 0.20;      // 20% for runs
        qreal output = 0.25;    // 25% for output
        qreal status = 0.05;    // 5% for status
    } proportions;
    
    // Calculated from widget size with improved distribution
    void calculate(const QSize& widgetSize, int blockCount);
    void calculateAdaptive(const QSize& widgetSize, int blockCount, qreal zoomFactor);
    
    // Force recalculation on next call
    void invalidateCache() { geometryCached = false; }
    
    // Get appropriate block size for given width and count
    qreal calculateBlockWidth(qreal availableWidth, int blockCount) const;
    
    // Check if layout needs update
    bool needsUpdate(const QSize& widgetSize, int blockCount) const;
};

// Viewport for virtualized rendering
struct Viewport {
    int startIndex = 0;
    int endIndex = 0;
    qreal scrollOffset = 0.0;
    qreal totalWidth = 0.0;
    qreal visibleWidth = 0.0;
    
    // Smooth scrolling
    qreal targetOffset = 0.0;
    bool isScrolling = false;
    
    bool isBlockVisible(int index, qreal blockWidth, qreal spacing) const;
    QPointF adjustPosition(const QPointF& pos) const;
};

// Smooth color transition helper
struct ColorTransition {
    QColor startColor;
    QColor endColor;
    QColor currentColor;
    double progress = 1.0;
    double duration = 200.0; // ms
    
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

// Animated arrow between regions - IMPROVED with smoother rendering
struct AnimatedArrow {
    QPointF start;
    QPointF end;
    QPointF control;  // For bezier curve
    double progress = 1.0;
    double opacity = 1.0;
    double targetOpacity = 1.0;
    QColor color{0, 143, 150};
    bool visible = false;
    bool useCurve = true;
    
    // Animation easing
    double easedProgress() const {
        // Smooth ease-in-out
        double t = progress;
        return t * t * (3.0 - 2.0 * t);
    }
    
    void render(QPainter* painter) const;
    void animate(double deltaTime, double speed = 1.0);
    
    // Smooth fade in/out
    void fadeIn() { targetOpacity = 1.0; }
    void fadeOut() { targetOpacity = 0.0; }
};

// Main visualization widget - IMPROVED for smooth animations
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
    
    // Set the animation controller
    void setController(AnimationController* controller);
    AnimationController* getController() { return controller; }
    
    // Initialize with data
    void setData(const QVector<double>& data);
    void clear();
    
    // Layout
    void recalculateLayout();
    void invalidateLayout(); // Force layout recalculation
    
    // Animation properties
    qreal cameraOffsetY() const { return m_cameraOffsetY; }
    void setCameraOffsetY(qreal offset);
    
    qreal ramScale() const { return m_ramScale; }
    void setRamScale(qreal scale);
    
    qreal introProgress() const { return m_introProgress; }
    void setIntroProgress(qreal progress);
    
    // Smooth highlight transition
    qreal highlightIntensity() const { return m_highlightIntensity; }
    void setHighlightIntensity(qreal intensity);
    
    // General transition progress for smooth state changes
    qreal transitionProgress() const { return m_transitionProgress; }
    void setTransitionProgress(qreal progress);
    
    // Zoom and viewport
    qreal zoomFactor() const { return m_zoomFactor; }
    void setZoomFactor(qreal zoom);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void setViewportOffset(qreal offset);
    qreal viewportOffset() const { return m_viewport.scrollOffset; }
    
    // Smooth viewport scrolling
    void smoothScrollTo(qreal offset, int durationMs = 300);
    
    // Scroll to make block visible
    void ensureBlockVisible(int blockIndex);
    void scrollToBlock(int blockIndex);
    
    // State access
    const QVector<MemoryBlock>& getDiskBlocks() const { return diskBlocks; }
    const QVector<MemoryBlock>& getRamBlocks() const { return ramBlocks; }
    
    // Export current state for history
    QVector<QPair<int, QPointF>> exportBlockPositions() const;
    void importBlockPositions(const QVector<QPair<int, QPointF>>& positions);
    
public slots:
    // Step handling
    void onStepStarted(const AnimationStep& step);
    void onStepProgress(const AnimationStep& step, double progress);
    void onStepCompleted(const AnimationStep& step);
    
    // Frame updates
    void onFrameUpdate(double deltaTime, double stepProgress);
    
    // Playback state
    void onPlaybackStarted();
    void onPlaybackStopped();

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
    // Rendering methods
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
    
    // Virtualized rendering
    void drawVisibleBlocks(QPainter& painter, const QVector<MemoryBlock>& blocks, 
                          const QRectF& area, const QString& areaName);
    bool shouldRenderBlock(int index) const;
    void updateViewport();
    
    // Step execution
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
    
    // Helpers
    QPointF getDiskBlockPosition(int index) const;
    QPointF getRAMBlockPosition(int index) const;
    QPointF getRunBlockPosition(int runIndex, int blockIndex) const;
    QPointF getOutputBlockPosition(int index) const;
    double getMaxValue() const;
    void updateBlockAnimations(double deltaTime);
    void sortRAMBlocks();
    
    // Animation controller
    AnimationController* controller = nullptr;
    
    // Layout
    VisualizerLayout layout;
    
    // Data blocks
    QVector<MemoryBlock> diskBlocks;
    QVector<MemoryBlock> ramBlocks;
    QVector<QVector<MemoryBlock>> runBlocks;
    QVector<MemoryBlock> outputBlocks;
    
    // Moving blocks (for animations)
    MovingBlockGroup movingBlocks;
    
    // Arrows
    QVector<AnimatedArrow> arrows;
    
    // Animation state
    AnimationStep currentStep;
    double currentProgress = 0.0;
    bool showPhaseOverlay = false;
    QString phaseText;
    double phaseOverlayProgress = 0.0;
    
    // Camera/transform
    qreal m_cameraOffsetY = 0.0;
    qreal m_ramScale = 0.8;              // Increased default scale
    qreal m_introProgress = 0.0;
    
    // Smooth transition properties
    qreal m_highlightIntensity = 0.0;
    qreal m_transitionProgress = 0.0;
    QPropertyAnimation* m_highlightAnim = nullptr;
    QPropertyAnimation* m_transitionAnim = nullptr;
    
    // Color transitions for smooth highlight effects
    ColorTransition m_diskHighlightColor;
    ColorTransition m_ramHighlightColor;
    QVector<ColorTransition> m_blockHighlightColors;
    
    // Zoom and viewport
    qreal m_zoomFactor = 1.0;
    qreal m_minZoom = 0.25;
    qreal m_maxZoom = 4.0;
    qreal m_zoomStep = 0.25;
    Viewport m_viewport;
    QPropertyAnimation* m_zoomAnim = nullptr;
    QPropertyAnimation* m_scrollAnim = nullptr;
    
    // Visual state
    bool diskHighlighted = false;
    bool ramHighlighted = false;
    int highlightedDiskBlock = -1;
    int highlightedRAMBlock = -1;
    QVector<int> comparedIndices;
    
    // Animations - using QPropertyAnimation for smooth transitions
    QPropertyAnimation* cameraAnim = nullptr;
    QPropertyAnimation* ramScaleAnim = nullptr;
    QPropertyAnimation* introAnim = nullptr;
    
    // Animation configuration
    int m_defaultAnimDuration = 250;     // Default transition duration (ms)
    int m_fastAnimDuration = 150;        // Fast transition duration (ms)
    int m_slowAnimDuration = 400;        // Slow transition duration (ms)
    
    // Timing - using high precision timer
    QElapsedTimer frameTimer;
    double lastFrameTime = 0.0;
    bool m_pendingUpdate = false;        // Prevent repaint spam
    
    // Cached geometry to prevent layout jitter
    QVector<QRectF> m_cachedDiskPositions;
    QVector<QRectF> m_cachedRamPositions;
    bool m_geometryDirty = true;
    
    // Stats
    int totalBlocks = 0;
    int runsGenerated = 0;
    int mergeProgress = 0;
};
