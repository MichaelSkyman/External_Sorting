#pragma once

#include <QObject>
#include <QQueue>
#include <QVector>
#include <QTimer>
#include <QEasingCurve>
#include <QMap>
#include <QVariant>
#include "animation_step.h"
#include "animation_timer.h"

/** @brief Adaptive speed profile controlling step pacing throughout playback. */
struct SpeedProfile {
    int introDelayMs   = 1200; ///< Step delay during introduction (very slow).
    int earlyDelayMs   = 800;  ///< Step delay during early sorting steps (slow).
    int normalDelayMs  = 400;  ///< Step delay during normal sorting flow.
    int fastDelayMs    = 150;  ///< Step delay during later merge phases (fast).
    int instantDelayMs = 50;   ///< Step delay for near-instant transitions.

    /// @name Transition Thresholds
    /// @{
    int introSteps  = 10;  ///< Steps at intro pace before switching to early pace.
    int earlySteps  = 30;  ///< Steps at early pace before switching to normal pace.
    int normalSteps = 100; ///< Steps at normal pace before switching to fast pace.
    /// @note Beyond @c normalSteps, fast pace applies.
    /// @}
};

/** @brief Snapshot of animation state used for history and rewind support. */
struct AnimationState {
    int stepIndex = 0;                           ///< Index of this snapshot in the step history.
    AnimationStep step;                          ///< The animation step this snapshot belongs to.
    QMap<QString, QVariant> visualState;         ///< Key-value snapshot of visualization properties.
    qint64 timestamp = 0;                        ///< Timestamp (ms) when this snapshot was captured.

    QVector<QPair<int, QPointF>> blockPositions; ///< Per-block positions at this point {blockId, pos}.

    QString currentPhase; ///< Active sort phase label at this snapshot.
    int runCount = 0;     ///< Number of sorted runs created at this point.

    /// @brief Returns true if the snapshot holds valid data.
    bool isValid() const { return stepIndex >= 0; }
};

/**
 * @brief Controls the animation step queue and drives playback.
 *
 * Manages a queue of AnimationStep objects, executes them with configurable
 * timing, and emits granular signals (stepStarted, stepProgress, stepCompleted)
 * for visualization components to respond to. Supports seeking, history
 * snapshots, and adaptive speed profiles.
 */
class AnimationController : public QObject
{
    Q_OBJECT

public:
    explicit AnimationController(QObject* parent = nullptr);
    ~AnimationController();

    /// @name Step Queue Management
    void enqueueStep(const AnimationStep& step);
    void enqueueSteps(const QVector<AnimationStep>& steps);
    void clearQueue();
    int queueSize() const { return stepQueue.size(); }
    bool hasSteps() const { return !stepQueue.empty(); }
    
    // Playback control
    void play();
    void pause();
    void resume();
    void stop();
    void stepForward();
    void stepBackward();
    void skipToEnd();
    
    // History and seek
    void seekToStep(int stepIndex);
    int getTotalSteps() const { return m_allSteps.size(); }
    int getCurrentStepIndex() const { return m_currentStepIndex; }
    const AnimationState& getStateAtStep(int stepIndex) const;
    void clearHistory();
    bool canStepBackward() const { return m_currentStepIndex > 0; }
    bool canStepForward() const { return m_currentStepIndex < m_allSteps.size() - 1; }
    
    // State snapshot for visualization sync
    void captureState(const QMap<QString, QVariant>& visualState);
    void setBlockPositions(const QVector<QPair<int, QPointF>>& positions);
    void setCurrentPhase(const QString& phase) { m_currentPhase = phase; }
    
    // Speed control
    void setSpeedMultiplier(double multiplier);
    double getSpeedMultiplier() const { return speedMultiplier; }
    void setAdaptiveSpeed(bool enabled) { adaptiveSpeedEnabled = enabled; }
    bool isAdaptiveSpeedEnabled() const { return adaptiveSpeedEnabled; }
    void setSpeedProfile(const SpeedProfile& profile) { speedProfile = profile; }
    
    // State queries
    bool isPlaying() const { return playing; }
    bool isPaused() const { return paused; }
    int getStepsExecuted() const { return stepsExecuted; }
    const AnimationStep& getCurrentStep() const { return currentStep; }
    double getCurrentProgress() const { return currentProgress; }
    
    // Animation frame timer
    AnimationTimer* getTimer() { return &frameTimer; }
    
    // Easing
    void setEasingCurve(QEasingCurve::Type type) { easingCurve.setType(type); }
    double getEasedProgress() const;
    
    // Intro sequence helpers
    void enqueueIntroSequence(const QVector<double>& diskData);
    void enqueueSortPhaseTransition();
    void enqueueMergePhaseTransition(int numRuns);

signals:
    // Step lifecycle
    void stepStarted(const AnimationStep& step);
    void stepProgress(const AnimationStep& step, double progress);
    void stepCompleted(const AnimationStep& step);
    
    // Queue status
    void queueSizeChanged(int size);
    void queueEmpty();
    
    // Playback status
    void playbackStarted();
    void playbackPaused();
    void playbackResumed();
    void playbackStopped();
    
    // History/seek signals
    void stepIndexChanged(int currentIndex, int totalSteps);
    void seekCompleted(int stepIndex);
    void stateRestored(const AnimationState& state);
    void totalStepsChanged(int totalSteps);
    
    // For visualizer updates
    void frameUpdate(double deltaTime, double stepProgress);
    void requestRepaint();

private slots:
    void onFrameUpdate(double deltaTime);
    void processNextStep();
    void onStepTimeout();

private:
    int calculateDelay(const AnimationStep& step);
    void startCurrentStep();
    void completeCurrentStep();
    void saveStateSnapshot();
    void restoreStateSnapshot(int stepIndex);

    QQueue<AnimationStep> stepQueue;         ///< Pending animation steps awaiting execution.
    AnimationStep currentStep;               ///< The step currently being played.

    QVector<AnimationStep> m_allSteps;       ///< Full list of all steps (for seeking/rewind).
    int m_currentStepIndex = -1;             ///< Index within m_allSteps of the current step.

    QVector<AnimationState> m_history;       ///< Recorded state snapshots for rewind.
    static constexpr int MaxHistorySize = 10000; ///< Maximum number of history snapshots kept.

    QMap<QString, QVariant> m_pendingVisualState;         ///< Buffered visual state for the next snapshot.
    QVector<QPair<int, QPointF>> m_pendingBlockPositions; ///< Buffered block positions for snapshot.
    QString m_currentPhase; ///< Active sort phase label.
    int m_runCount = 0;     ///< Number of runs created so far.

    AnimationTimer frameTimer; ///< Frame-rate timer driving progress interpolation.
    QTimer stepTimer;          ///< One-shot timer for step duration tracking.
    QEasingCurve easingCurve;  ///< Easing curve applied to step progress values.

    bool playing        = false; ///< True while playback is active.
    bool paused         = false; ///< True when playback is paused.
    bool processingStep = false; ///< True while a step is being executed.

    int    stepsExecuted   = 0;   ///< Total number of steps executed in this session.
    double currentProgress = 0.0; ///< Progress (0–1) through the current step.
    double stepElapsedMs   = 0.0; ///< Elapsed time in the current step (ms).

    double speedMultiplier      = 1.0;  ///< Global speed multiplier (e.g., 2.0 = double speed).
    bool   adaptiveSpeedEnabled = true; ///< When true, uses SpeedProfile pacing.
    SpeedProfile speedProfile;          ///< Speed profile controlling per-phase delays.
};
