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

// Animation speed presets
struct SpeedProfile {
    int introDelayMs = 1200;      // Very slow for intro
    int earlyDelayMs = 800;       // Slow for early steps
    int normalDelayMs = 400;      // Normal speed
    int fastDelayMs = 150;        // Fast for later merge
    int instantDelayMs = 50;      // Near instant
    
    // Transition points
    int introSteps = 10;          // First N steps are intro pace
    int earlySteps = 30;          // Next N steps are early pace
    int normalSteps = 100;        // Then normal pace
    // Beyond that is fast pace
};

// Snapshot of animation state for history/rewind
struct AnimationState {
    int stepIndex = 0;                          // Which step this state represents
    AnimationStep step;                         // The animation step itself
    QMap<QString, QVariant> visualState;        // Snapshot of visualization state
    qint64 timestamp = 0;                       // When this state was captured
    
    // Block positions at this point
    QVector<QPair<int, QPointF>> blockPositions;  // {blockId, position}
    
    // Phase information
    QString currentPhase;
    int runCount = 0;
    
    bool isValid() const { return stepIndex >= 0; }
};

class AnimationController : public QObject
{
    Q_OBJECT

public:
    explicit AnimationController(QObject* parent = nullptr);
    ~AnimationController();
    
    // Step queue management
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

    // Step queue
    QQueue<AnimationStep> stepQueue;
    AnimationStep currentStep;
    
    // All steps (for seeking/rewind)
    QVector<AnimationStep> m_allSteps;
    int m_currentStepIndex = -1;
    
    // History of animation states
    QVector<AnimationState> m_history;
    static constexpr int MaxHistorySize = 10000;  // Limit history to prevent memory issues
    
    // Current visualization state for snapshot
    QMap<QString, QVariant> m_pendingVisualState;
    QVector<QPair<int, QPointF>> m_pendingBlockPositions;
    QString m_currentPhase;
    int m_runCount = 0;
    
    // Timing
    AnimationTimer frameTimer;
    QTimer stepTimer;
    QEasingCurve easingCurve;
    
    // State
    bool playing = false;
    bool paused = false;
    bool processingStep = false;
    
    // Progress
    int stepsExecuted = 0;
    double currentProgress = 0.0;
    double stepElapsedMs = 0.0;
    
    // Speed control
    double speedMultiplier = 1.0;
    bool adaptiveSpeedEnabled = true;
    SpeedProfile speedProfile;
};
