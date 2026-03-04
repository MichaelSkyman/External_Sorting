#ifndef FULLSCREEN_VISUALIZER_H
#define FULLSCREEN_VISUALIZER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QToolButton>
#include "external_sort_canvas.h"
#include "../animation/step_aggregator.h"

/**
 * @brief FullscreenVisualizer - Minimal UI wrapper for fullscreen animation
 * 
 * Layout:
 * ┌──────────────────────────────────────────────┐
 * │ [Progress Bar] ─────────────────────────────│
 * ├──────────────────────────────────────────────┤
 * │ [◀] [⏸] [▶] [Step] │ Speed: [═══●═══] 1.0x │ << Control Bar
 * ├──────────────────────────────────────────────┤
 * │                                              │
 * │                                              │
 * │          FULLSCREEN ANIMATION CANVAS         │
 * │                                              │
 * │             (fills remaining space)          │
 * │                                              │
 * │                                              │
 * └──────────────────────────────────────────────┘
 * 
 * Control bar auto-hides during animation (optional).
 * Progress bar shows overall sorting progress.
 * Canvas dynamically resizes with window.
 */
class FullscreenVisualizer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal controlBarOpacity READ controlBarOpacity WRITE setControlBarOpacity)

public:
    explicit FullscreenVisualizer(QWidget* parent = nullptr);
    ~FullscreenVisualizer();
    
    /// @brief Returns the sort animation canvas.
    ExternalSortCanvas* canvas() const { return m_canvas; }
    /// @brief Returns the animation engine driving the canvas.
    AnimationEngine* engine() const { return m_engine; }
    /// @brief Returns the step aggregator batching raw algorithm operations.
    StepAggregator* aggregator() const { return m_aggregator; }

    /// @brief Populates the canvas with data values and resets state.
    void setData(const QVector<double>& data);
    /// @brief Clears all canvas state and resets playback.
    void clear();

    /// @brief Starts or resumes animation playback.
    void play();
    /// @brief Pauses animation playback at the current step.
    void pause();
    /// @brief Stops animation playback and resets to the first step.
    void stop();
    /// @brief Advances playback by one step.
    void stepForward();
    /// @brief Rewinds playback by one step.
    void stepBackward();

    /// @brief Sets the playback speed multiplier (range 0.1–10x).
    void setSpeed(double multiplier);
    /// @brief Returns the current playback speed multiplier.
    double speed() const;

    /// @brief Updates the progress indicator to @p current out of @p total steps.
    void setProgress(int current, int total);
    /// @brief Returns the current step progress index.
    int progress() const;

    /// @brief Shows the control bar and resets the auto-hide timer.
    void showControlBar();
    /// @brief Hides the control bar immediately.
    void hideControlBar();
    /// @brief Enables or disables automatic control bar hiding.
    void setAutoHideEnabled(bool enabled) { m_autoHideEnabled = enabled; }
    /// @brief Returns true when automatic control bar hiding is active.
    bool autoHideEnabled() const { return m_autoHideEnabled; }

    /// @brief Returns the current control bar opacity (0–1).
    qreal controlBarOpacity() const { return m_controlBarOpacity; }
    /// @brief Sets the control bar opacity for fade animations.
    void setControlBarOpacity(qreal opacity);
    
signals:
    void backRequested();
    void playbackStarted();
    void playbackPaused();
    void playbackStopped();
    void stepChanged(int current, int total);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onPlayPauseClicked();
    void onStepForwardClicked();
    void onStepBackwardClicked();
    void onSpeedChanged(int value);
    void onEngineStepIndexChanged(int current, int total);
    void onEngineQueueEmpty();
    void startAutoHideTimer();

private:
    void setupUI();
    void setupEngine();
    void updateSpeedLabel();
    void updateStepLabel();
    
    ExternalSortCanvas* m_canvas     = nullptr; ///< The sort animation canvas.
    AnimationEngine*    m_engine     = nullptr; ///< Animation engine driving the canvas.
    StepAggregator*     m_aggregator = nullptr; ///< Step aggregator batching raw operations.

    QWidget*      m_controlBar  = nullptr; ///< Control bar widget.
    QProgressBar* m_progressBar = nullptr; ///< Overall sort progress bar.

    QToolButton* m_playPauseBtn = nullptr; ///< Play/pause toggle button.
    QToolButton* m_stepBackBtn  = nullptr; ///< Step-backward button.
    QToolButton* m_stepFwdBtn   = nullptr; ///< Step-forward button.
    QToolButton* m_backBtn      = nullptr; ///< Back-to-setup button.

    QSlider* m_speedSlider = nullptr; ///< Speed control slider.
    QLabel*  m_speedLabel  = nullptr; ///< Speed value label.

    QLabel* m_stepLabel = nullptr; ///< Step counter label.

    QTimer*             m_autoHideTimer    = nullptr; ///< Timer triggering control bar auto-hide.
    bool                m_autoHideEnabled  = true;    ///< When true, control bar auto-hides.
    qreal               m_controlBarOpacity = 1.0;   ///< Current control bar opacity (0–1).
    QPropertyAnimation* m_controlBarAnim   = nullptr; ///< Fade animation for the control bar.

    bool m_isPlaying   = false; ///< True while playback is in progress.
    int  m_currentStep = 0;     ///< Current step index.
    int  m_totalSteps  = 0;     ///< Total number of animation steps.
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline FullscreenVisualizer::FullscreenVisualizer(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setupUI();
    setupEngine();
    
    // Mouse tracking for auto-hide
    setMouseTracking(true);
}

inline FullscreenVisualizer::~FullscreenVisualizer()
{
}

inline void FullscreenVisualizer::setupUI()
{
    // Main layout - vertical, no margins for fullscreen feel
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // === PROGRESS BAR (top) ===
    m_progressBar = new QProgressBar(this);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setTextVisible(false);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            background-color: #1a1c20;
            border: none;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00b4d8, stop:1 #0077b6);
        }
    )");
    mainLayout->addWidget(m_progressBar);
    
    // === CONTROL BAR ===
    m_controlBar = new QWidget(this);
    m_controlBar->setFixedHeight(45);
    m_controlBar->setStyleSheet(R"(
        QWidget {
            background-color: rgba(30, 32, 36, 230);
            border-bottom: 1px solid #2a2c30;
        }
        QToolButton {
            background: transparent;
            border: 1px solid #3a3c40;
            border-radius: 4px;
            color: white;
            padding: 6px 12px;
            font-size: 14px;
        }
        QToolButton:hover {
            background-color: #00b4d8;
            border-color: #00b4d8;
        }
        QToolButton:pressed {
            background-color: #0077b6;
        }
        QToolButton:checked {
            background-color: #ff9800;
            border-color: #ff9800;
        }
        QLabel {
            color: #aaa;
            font-size: 11px;
        }
        QSlider::groove:horizontal {
            background: #2a2c30;
            height: 4px;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #00b4d8;
            width: 14px;
            margin: -5px 0;
            border-radius: 7px;
        }
        QSlider::sub-page:horizontal {
            background: #00b4d8;
            border-radius: 2px;
        }
    )");
    
    QHBoxLayout* controlLayout = new QHBoxLayout(m_controlBar);
    controlLayout->setContentsMargins(12, 6, 12, 6);
    controlLayout->setSpacing(8);
    
    // Back button
    m_backBtn = new QToolButton(m_controlBar);
    m_backBtn->setText("← Back");
    m_backBtn->setToolTip("Return to setup");
    connect(m_backBtn, &QToolButton::clicked, this, &FullscreenVisualizer::backRequested);
    controlLayout->addWidget(m_backBtn);
    
    controlLayout->addSpacing(20);
    
    // Step backward
    m_stepBackBtn = new QToolButton(m_controlBar);
    m_stepBackBtn->setText("⏮");
    m_stepBackBtn->setToolTip("Previous step");
    connect(m_stepBackBtn, &QToolButton::clicked, this, &FullscreenVisualizer::onStepBackwardClicked);
    controlLayout->addWidget(m_stepBackBtn);
    
    // Play/Pause
    m_playPauseBtn = new QToolButton(m_controlBar);
    m_playPauseBtn->setText("▶");
    m_playPauseBtn->setToolTip("Play/Pause");
    m_playPauseBtn->setCheckable(true);
    connect(m_playPauseBtn, &QToolButton::clicked, this, &FullscreenVisualizer::onPlayPauseClicked);
    controlLayout->addWidget(m_playPauseBtn);
    
    // Step forward
    m_stepFwdBtn = new QToolButton(m_controlBar);
    m_stepFwdBtn->setText("⏭");
    m_stepFwdBtn->setToolTip("Next step");
    connect(m_stepFwdBtn, &QToolButton::clicked, this, &FullscreenVisualizer::onStepForwardClicked);
    controlLayout->addWidget(m_stepFwdBtn);
    
    controlLayout->addSpacing(20);
    
    // Speed label
    QLabel* speedText = new QLabel("Speed:", m_controlBar);
    controlLayout->addWidget(speedText);
    
    // Speed slider
    m_speedSlider = new QSlider(Qt::Horizontal, m_controlBar);
    m_speedSlider->setRange(1, 20);  // 0.1x to 2.0x in 0.1 steps
    m_speedSlider->setValue(10);     // Default 1.0x
    m_speedSlider->setFixedWidth(120);
    connect(m_speedSlider, &QSlider::valueChanged, this, &FullscreenVisualizer::onSpeedChanged);
    controlLayout->addWidget(m_speedSlider);
    
    m_speedLabel = new QLabel("1.0x", m_controlBar);
    m_speedLabel->setFixedWidth(40);
    m_speedLabel->setStyleSheet("color: #00b4d8; font-weight: bold;");
    controlLayout->addWidget(m_speedLabel);
    
    controlLayout->addStretch();
    
    // Step counter
    m_stepLabel = new QLabel("Step: 0 / 0", m_controlBar);
    m_stepLabel->setStyleSheet("color: #888; font-size: 12px;");
    controlLayout->addWidget(m_stepLabel);
    
    mainLayout->addWidget(m_controlBar);
    
    // === ANIMATION CANVAS (dominant, fills remaining space) ===
    m_canvas = new ExternalSortCanvas(this);
    mainLayout->addWidget(m_canvas, 1);  // stretch factor 1 = fills available space
    
    // Auto-hide timer
    m_autoHideTimer = new QTimer(this);
    m_autoHideTimer->setSingleShot(true);
    connect(m_autoHideTimer, &QTimer::timeout, this, &FullscreenVisualizer::hideControlBar);
    
    // Control bar fade animation
    m_controlBarAnim = new QPropertyAnimation(this, "controlBarOpacity", this);
    m_controlBarAnim->setEasingCurve(QEasingCurve::InOutQuad);
    m_controlBarAnim->setDuration(300);
}

inline void FullscreenVisualizer::setupEngine()
{
    m_engine = new AnimationEngine(this);
    m_aggregator = new StepAggregator(this);
    
    // Connect engine to canvas
    m_canvas->setEngine(m_engine);
    
    // Connect engine signals
    connect(m_engine, &AnimationEngine::stepIndexChanged,
            this, &FullscreenVisualizer::onEngineStepIndexChanged);
    connect(m_engine, &AnimationEngine::queueEmpty,
            this, &FullscreenVisualizer::onEngineQueueEmpty);
    connect(m_engine, &AnimationEngine::playbackStarted,
            this, &FullscreenVisualizer::playbackStarted);
    connect(m_engine, &AnimationEngine::playbackStopped,
            this, &FullscreenVisualizer::playbackStopped);
}

inline void FullscreenVisualizer::setData(const QVector<double>& data)
{
    m_canvas->setData(data);
    m_progressBar->setValue(0);
    updateStepLabel();
}

inline void FullscreenVisualizer::clear()
{
    m_engine->stop();
    m_engine->clearQueue();
    m_canvas->clear();
    m_progressBar->setValue(0);
    m_currentStep = 0;
    m_totalSteps = 0;
    updateStepLabel();
}

inline void FullscreenVisualizer::play()
{
    m_engine->start();
    m_isPlaying = true;
    m_playPauseBtn->setText("⏸");
    m_playPauseBtn->setChecked(true);
    emit playbackStarted();
    
    if (m_autoHideEnabled) {
        startAutoHideTimer();
    }
}

inline void FullscreenVisualizer::pause()
{
    m_engine->pause();
    m_isPlaying = false;
    m_playPauseBtn->setText("▶");
    m_playPauseBtn->setChecked(false);
    emit playbackPaused();
    
    showControlBar();
}

inline void FullscreenVisualizer::stop()
{
    m_engine->stop();
    m_isPlaying = false;
    m_playPauseBtn->setText("▶");
    m_playPauseBtn->setChecked(false);
    emit playbackStopped();
    
    showControlBar();
}

inline void FullscreenVisualizer::stepForward()
{
    m_engine->stepForward();
}

inline void FullscreenVisualizer::stepBackward()
{
    m_engine->stepBackward();
}

inline void FullscreenVisualizer::setSpeed(double multiplier)
{
    m_engine->setSpeedMultiplier(multiplier);
    int sliderValue = static_cast<int>(multiplier * 10);
    m_speedSlider->setValue(qBound(1, sliderValue, 20));
    updateSpeedLabel();
}

inline double FullscreenVisualizer::speed() const
{
    return m_engine->speedMultiplier();
}

inline void FullscreenVisualizer::setProgress(int current, int total)
{
    int percent = total > 0 ? (current * 100) / total : 0;
    m_progressBar->setValue(percent);
}

inline int FullscreenVisualizer::progress() const
{
    return m_progressBar->value();
}

inline void FullscreenVisualizer::showControlBar()
{
    m_autoHideTimer->stop();
    
    m_controlBarAnim->stop();
    m_controlBarAnim->setStartValue(m_controlBarOpacity);
    m_controlBarAnim->setEndValue(1.0);
    m_controlBarAnim->start();
}

inline void FullscreenVisualizer::hideControlBar()
{
    if (!m_isPlaying) return;  // Only hide during playback
    
    m_controlBarAnim->stop();
    m_controlBarAnim->setStartValue(m_controlBarOpacity);
    m_controlBarAnim->setEndValue(0.3);  // Semi-transparent, not fully hidden
    m_controlBarAnim->start();
}

inline void FullscreenVisualizer::setControlBarOpacity(qreal opacity)
{
    if (!qFuzzyCompare(m_controlBarOpacity, opacity)) {
        m_controlBarOpacity = opacity;
        m_controlBar->setWindowOpacity(opacity);
        
        // Also adjust progress bar
        QString style = QString(R"(
            QProgressBar {
                background-color: rgba(26, 28, 32, %1);
                border: none;
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 rgba(0, 180, 216, %1), stop:1 rgba(0, 119, 182, %1));
            }
        )").arg(static_cast<int>(opacity * 255));
        m_progressBar->setStyleSheet(style);
    }
}

inline void FullscreenVisualizer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

inline void FullscreenVisualizer::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    
    // Show control bar on mouse movement
    showControlBar();
    
    if (m_autoHideEnabled && m_isPlaying) {
        startAutoHideTimer();
    }
}

inline void FullscreenVisualizer::enterEvent(QEnterEvent* event)
{
    QWidget::enterEvent(event);
    showControlBar();
}

inline void FullscreenVisualizer::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    
    if (m_autoHideEnabled && m_isPlaying) {
        hideControlBar();
    }
}

inline void FullscreenVisualizer::onPlayPauseClicked()
{
    if (m_isPlaying) {
        pause();
    } else {
        play();
    }
}

inline void FullscreenVisualizer::onStepForwardClicked()
{
    stepForward();
    showControlBar();
}

inline void FullscreenVisualizer::onStepBackwardClicked()
{
    stepBackward();
    showControlBar();
}

inline void FullscreenVisualizer::onSpeedChanged(int value)
{
    double multiplier = value / 10.0;
    m_engine->setSpeedMultiplier(multiplier);
    updateSpeedLabel();
}

inline void FullscreenVisualizer::onEngineStepIndexChanged(int current, int total)
{
    m_currentStep = current;
    m_totalSteps = total;
    updateStepLabel();
    setProgress(current, total);
    emit stepChanged(current, total);
}

inline void FullscreenVisualizer::onEngineQueueEmpty()
{
    stop();
    m_progressBar->setValue(100);
}

inline void FullscreenVisualizer::startAutoHideTimer()
{
    m_autoHideTimer->start(3000);  // Hide after 3 seconds of inactivity
}

inline void FullscreenVisualizer::updateSpeedLabel()
{
    double speed = m_engine->speedMultiplier();
    m_speedLabel->setText(QString("%1x").arg(speed, 0, 'f', 1));
}

inline void FullscreenVisualizer::updateStepLabel()
{
    m_stepLabel->setText(QString("Step: %1 / %2").arg(m_currentStep).arg(m_totalSteps));
}

#endif // FULLSCREEN_VISUALIZER_H
