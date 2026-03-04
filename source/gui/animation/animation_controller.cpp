#include "animation_controller.h"
#include <QtMath>
#include <QDateTime>

AnimationController::AnimationController(QObject* parent)
    : QObject(parent)
    , easingCurve(QEasingCurve::InOutCubic)
{
    // Connect frame timer to update loop
    connect(&frameTimer, &AnimationTimer::frame, 
            this, &AnimationController::onFrameUpdate);
    
    // Step timer for step completion
    stepTimer.setSingleShot(true);
    connect(&stepTimer, &QTimer::timeout, 
            this, &AnimationController::onStepTimeout);
    
    // Set up smoother animation timing
    frameTimer.setTargetFps(60);
    
    // Initialize adaptive speed profile for educational pacing
    speedProfile.introSteps = 15;     // Longer intro observation period
    speedProfile.earlySteps = 40;     // More early steps at slower pace
    speedProfile.normalSteps = 120;   // Normal pace for middle section
    speedProfile.introDelayMs = 1500; // Very slow intro
    speedProfile.earlyDelayMs = 900;  // Slow early steps
    speedProfile.normalDelayMs = 500; // Normal speed
    speedProfile.fastDelayMs = 200;   // Faster for repetitive operations
}

AnimationController::~AnimationController()
{
    stop();
}

void AnimationController::enqueueStep(const AnimationStep& step)
{
    stepQueue.enqueue(step);
    m_allSteps.append(step);
    emit queueSizeChanged(stepQueue.size());
    emit totalStepsChanged(m_allSteps.size());
}

void AnimationController::enqueueSteps(const QVector<AnimationStep>& steps)
{
    for (const auto& step : steps) {
        stepQueue.enqueue(step);
        m_allSteps.append(step);
    }
    emit queueSizeChanged(stepQueue.size());
    emit totalStepsChanged(m_allSteps.size());
}

void AnimationController::clearQueue()
{
    stepQueue.clear();
    m_allSteps.clear();
    m_history.clear();
    m_currentStepIndex = -1;
    emit queueSizeChanged(0);
    emit totalStepsChanged(0);
}

void AnimationController::play()
{
    if (!playing) {
        playing = true;
        paused = false;
        stepsExecuted = 0;
        frameTimer.start();
        emit playbackStarted();
        processNextStep();
    }
}

void AnimationController::pause()
{
    if (playing && !paused) {
        paused = true;
        stepTimer.stop();
        emit playbackPaused();
    }
}

void AnimationController::resume()
{
    if (playing && paused) {
        paused = false;
        
        // Resume step timer with remaining time
        if (processingStep) {
            int remainingMs = static_cast<int>(
                (1.0 - currentProgress) * currentStep.durationMs / speedMultiplier);
            if (remainingMs > 0) {
                stepTimer.start(remainingMs);
            }
        }
        emit playbackResumed();
    }
}

void AnimationController::stop()
{
    playing = false;
    paused = false;
    processingStep = false;
    currentProgress = 0.0;
    stepElapsedMs = 0.0;
    stepTimer.stop();
    frameTimer.stop();
    emit playbackStopped();
}

void AnimationController::stepForward()
{
    if (!stepQueue.empty()) {
        // If currently processing, complete immediately
        if (processingStep) {
            stepTimer.stop();
            completeCurrentStep();
        }
        // Start next step in paused mode
        processNextStep();
    } else if (m_currentStepIndex < m_allSteps.size() - 1) {
        // Use history when queue is empty
        seekToStep(m_currentStepIndex + 1);
    }
}

void AnimationController::stepBackward()
{
    if (m_currentStepIndex > 0) {
        // Stop current playback
        if (processingStep) {
            stepTimer.stop();
            processingStep = false;
        }
        
        seekToStep(m_currentStepIndex - 1);
    }
}

void AnimationController::seekToStep(int stepIndex)
{
    if (stepIndex < 0 || stepIndex >= m_allSteps.size()) {
        return;
    }
    
    // Stop current animation
    bool wasPlaying = playing;
    if (processingStep) {
        stepTimer.stop();
        processingStep = false;
    }
    
    // Check if we have a state snapshot for this step
    if (stepIndex < m_history.size() && m_history[stepIndex].isValid()) {
        restoreStateSnapshot(stepIndex);
    } else {
        // No snapshot available - just jump to step
        m_currentStepIndex = stepIndex;
        currentStep = m_allSteps[stepIndex];
        stepsExecuted = stepIndex;
    }
    
    emit stepIndexChanged(m_currentStepIndex, m_allSteps.size());
    emit seekCompleted(m_currentStepIndex);
    emit stepStarted(currentStep);
    emit requestRepaint();
    
    // If was playing, continue from new position
    if (wasPlaying && !paused) {
        // Rebuild queue from current position
        stepQueue.clear();
        for (int i = stepIndex + 1; i < m_allSteps.size(); ++i) {
            stepQueue.enqueue(m_allSteps[i]);
        }
        emit queueSizeChanged(stepQueue.size());
        startCurrentStep();
    }
}

const AnimationState& AnimationController::getStateAtStep(int stepIndex) const
{
    static AnimationState invalidState;
    if (stepIndex >= 0 && stepIndex < m_history.size()) {
        return m_history[stepIndex];
    }
    return invalidState;
}

void AnimationController::clearHistory()
{
    m_history.clear();
}

void AnimationController::captureState(const QMap<QString, QVariant>& visualState)
{
    m_pendingVisualState = visualState;
}

void AnimationController::setBlockPositions(const QVector<QPair<int, QPointF>>& positions)
{
    m_pendingBlockPositions = positions;
}

void AnimationController::saveStateSnapshot()
{
    if (m_currentStepIndex < 0) return;
    
    AnimationState state;
    state.stepIndex = m_currentStepIndex;
    state.step = currentStep;
    state.visualState = m_pendingVisualState;
    state.blockPositions = m_pendingBlockPositions;
    state.currentPhase = m_currentPhase;
    state.runCount = m_runCount;
    state.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    // Ensure history vector is large enough
    while (m_history.size() <= m_currentStepIndex) {
        m_history.append(AnimationState());
    }
    
    // Store snapshot (but limit total history)
    if (m_currentStepIndex < MaxHistorySize) {
        m_history[m_currentStepIndex] = state;
    }
}

void AnimationController::restoreStateSnapshot(int stepIndex)
{
    if (stepIndex < 0 || stepIndex >= m_history.size()) return;
    
    const AnimationState& state = m_history[stepIndex];
    if (!state.isValid()) return;
    
    m_currentStepIndex = stepIndex;
    currentStep = state.step;
    stepsExecuted = stepIndex;
    m_currentPhase = state.currentPhase;
    m_runCount = state.runCount;
    
    emit stateRestored(state);
}

void AnimationController::skipToEnd()
{
    while (!stepQueue.empty()) {
        currentStep = stepQueue.dequeue();
        stepsExecuted++;
        emit stepCompleted(currentStep);
    }
    emit queueSizeChanged(0);
    emit queueEmpty();
}

void AnimationController::setSpeedMultiplier(double multiplier)
{
    speedMultiplier = qBound(0.1, multiplier, 10.0);
}

double AnimationController::getEasedProgress() const
{
    return easingCurve.valueForProgress(currentProgress);
}

void AnimationController::enqueueIntroSequence(const QVector<double>& diskData)
{
    // Build cinematic intro
    QVector<AnimationStep> intro;
    
    // 1. Show disk
    auto diskStep = AnimationStep::createIntroDisk(diskData);
    diskStep.durationMs = 2000;
    intro.append(diskStep);
    
    // 2. Pause to observe
    intro.append(AnimationStep::createPause(1500, "Observe disk data"));
    
    // 3. Highlight RAM
    auto ramStep = AnimationStep::createHighlightRAM();
    ramStep.durationMs = 1200;
    intro.append(ramStep);
    
    // 4. Zoom into RAM
    intro.append(AnimationStep::createZoomRAM(1.0f));
    
    // 5. Camera focus transition
    intro.append(AnimationStep::createCameraFocus(QPointF(0, -50)));
    
    // 6. Pause before starting
    intro.append(AnimationStep::createPause(1000, "Ready to begin sorting"));
    
    // 7. Phase transition
    intro.append(AnimationStep::createPhaseTransition("Run Generation Phase"));
    
    enqueueSteps(intro);
}

void AnimationController::enqueueSortPhaseTransition()
{
    enqueueStep(AnimationStep::createPause(1200, "Preparing to sort in RAM"));
    enqueueStep(AnimationStep::createPhaseTransition("Sorting in Memory"));
}

void AnimationController::enqueueMergePhaseTransition(int numRuns)
{
    enqueueStep(AnimationStep::createPause(1500, "All runs generated"));
    enqueueStep(AnimationStep::createPhaseTransition("Merge Phase"));
    enqueueStep(AnimationStep::createMergeStart(numRuns));
}

void AnimationController::onFrameUpdate(double deltaTime)
{
    if (!playing || paused) return;
    
    if (processingStep) {
        // Update step progress with smooth interpolation
        stepElapsedMs += deltaTime * 1000.0;
        double duration = currentStep.durationMs / speedMultiplier;
        
        // Clamp progress to prevent overshooting
        double rawProgress = qMin(1.0, stepElapsedMs / duration);
        currentProgress = rawProgress;
        
        // Get eased progress for smooth visual updates
        double easedProg = getEasedProgress();
        
        // Emit progress signals
        emit stepProgress(currentStep, easedProg);
        emit frameUpdate(deltaTime, easedProg);
        emit requestRepaint();
    }
}

void AnimationController::processNextStep()
{
    if (stepQueue.empty()) {
        processingStep = false;
        emit queueEmpty();
        return;
    }
    
    currentStep = stepQueue.dequeue();
    emit queueSizeChanged(stepQueue.size());
    
    startCurrentStep();
}

void AnimationController::startCurrentStep()
{
    processingStep = true;
    currentProgress = 0.0;
    stepElapsedMs = 0.0;
    m_currentStepIndex = stepsExecuted;
    
    emit stepStarted(currentStep);
    emit stepIndexChanged(m_currentStepIndex, m_allSteps.size());
    
    // Calculate effective duration
    int effectiveDelay = calculateDelay(currentStep);
    
    // Start timer for step completion
    if (!paused) {
        stepTimer.start(effectiveDelay);
    }
}

void AnimationController::completeCurrentStep()
{
    // Save state snapshot before completing
    saveStateSnapshot();
    
    processingStep = false;
    currentProgress = 1.0;
    stepsExecuted++;
    
    emit stepCompleted(currentStep);
    emit requestRepaint();
    
    // Handle delay after step
    if (currentStep.delayAfterMs > 0 && !paused) {
        QTimer::singleShot(currentStep.delayAfterMs / speedMultiplier, 
                          this, &AnimationController::processNextStep);
    } else {
        processNextStep();
    }
}

void AnimationController::onStepTimeout()
{
    if (processingStep && !paused) {
        completeCurrentStep();
    }
}

int AnimationController::calculateDelay(const AnimationStep& step)
{
    int baseDelay = step.durationMs;
    
    // Apply adaptive speed if enabled
    // This creates a natural pacing that slows down for educational clarity
    // at the beginning and speeds up for repetitive operations later
    if (adaptiveSpeedEnabled) {
        int totalExecuted = stepsExecuted;
        
        if (totalExecuted < speedProfile.introSteps) {
            // Phase 1: Very slow intro - let user observe initial setup
            // Multiplier: 0.5x (slower)
            baseDelay = qMax(baseDelay, speedProfile.introDelayMs);
        } else if (totalExecuted < speedProfile.introSteps + speedProfile.earlySteps) {
            // Phase 2: Slow early execution - educational clarity
            // Multiplier: 0.75x
            baseDelay = qMax(baseDelay, speedProfile.earlyDelayMs);
        } else if (totalExecuted < speedProfile.introSteps + speedProfile.earlySteps + speedProfile.normalSteps) {
            // Phase 3: Normal speed - comfortable viewing
            // Multiplier: 1.0x
            baseDelay = qMax(baseDelay, speedProfile.normalDelayMs);
        } else {
            // Phase 4: Fast later execution - repetitive operations
            // Multiplier: 1.5x+ (faster)
            baseDelay = qMin(baseDelay, speedProfile.fastDelayMs);
        }
        
        // Special handling for specific step types
        switch (step.type) {
            case StepType::PhaseTransition:
                // Phase transitions should always be visible
                baseDelay = qMax(baseDelay, 1200);
                break;
            case StepType::Pause:
                // Pauses should use their specified duration
                baseDelay = step.durationMs;
                break;
            case StepType::MergeStep:
            case StepType::WriteOutput:
                // These repetitive steps can be faster after initial viewing
                if (totalExecuted > speedProfile.introSteps + speedProfile.earlySteps) {
                    baseDelay = qMin(baseDelay, speedProfile.fastDelayMs);
                }
                break;
            default:
                break;
        }
    }
    
    // Apply user speed multiplier
    int scaledDelay = static_cast<int>(baseDelay / speedMultiplier);
    
    // Ensure minimum delay for smooth animation (at least 1 frame at 60fps)
    return qMax(16, scaledDelay);
}
