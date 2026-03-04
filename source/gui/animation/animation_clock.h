#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <functional>

/**
 * @brief Centralized animation clock with fixed timestep updates.
 * 
 * This class provides a single, stable animation driver to prevent
 * jitter caused by multiple timers running simultaneously.
 * Uses fixed timestep with interpolation for smooth rendering.
 */
class AnimationClock : public QObject
{
    Q_OBJECT

public:
    explicit AnimationClock(QObject* parent = nullptr)
        : QObject(parent)
        , m_timer(new QTimer(this))
        , m_running(false)
        , m_targetFps(60)
        , m_fixedDeltaTime(1.0 / 60.0)
        , m_accumulator(0.0)
        , m_interpolation(0.0)
    {
        m_timer->setTimerType(Qt::PreciseTimer);
        connect(m_timer, &QTimer::timeout, this, &AnimationClock::tick);
    }
    
    // Configuration
    void setTargetFps(int fps) {
        m_targetFps = qBound(30, fps, 144);
        m_fixedDeltaTime = 1.0 / m_targetFps;
        if (m_running) {
            m_timer->setInterval(1000 / m_targetFps);
        }
    }
    
    int targetFps() const { return m_targetFps; }
    double fixedDeltaTime() const { return m_fixedDeltaTime; }
    
    // State queries
    bool isRunning() const { return m_running; }
    double elapsedTime() const { return m_elapsed.elapsed() / 1000.0; }
    qint64 frameCount() const { return m_frameCount; }
    
    // Interpolation factor for smooth rendering (0-1)
    // Use this to interpolate between physics/logic states
    double interpolation() const { return m_interpolation; }
    
    // Actual delta time since last frame (for variable timestep needs)
    double actualDeltaTime() const { return m_actualDeltaTime; }
    
    // Speed multiplier affects logical updates
    void setSpeedMultiplier(double multiplier) {
        m_speedMultiplier = qBound(0.1, multiplier, 10.0);
    }
    double speedMultiplier() const { return m_speedMultiplier; }
    
    // Phase-aware speed (for educational pacing)
    enum class Phase {
        Intro,      // Very slow for initial observation
        Early,      // Slow for learning
        Normal,     // Standard pace
        Fast        // Accelerated for repetitive operations
    };
    
    void setCurrentPhase(Phase phase) {
        m_currentPhase = phase;
    }
    
    double getPhaseMultiplier() const {
        switch (m_currentPhase) {
            case Phase::Intro:  return 0.5;
            case Phase::Early:  return 0.75;
            case Phase::Normal: return 1.0;
            case Phase::Fast:   return 1.5;
            default:            return 1.0;
        }
    }
    
    double effectiveSpeed() const {
        return m_speedMultiplier * getPhaseMultiplier();
    }

public slots:
    void start() {
        if (!m_running) {
            m_running = true;
            m_frameCount = 0;
            m_lastTime = 0;
            m_accumulator = 0.0;
            m_elapsed.start();
            m_timer->start(1000 / m_targetFps);
            emit started();
        }
    }
    
    void stop() {
        if (m_running) {
            m_running = false;
            m_timer->stop();
            emit stopped();
        }
    }
    
    void pause() {
        if (m_running) {
            m_timer->stop();
            m_pauseTime = m_elapsed.elapsed();
            emit paused();
        }
    }
    
    void resume() {
        if (m_running) {
            // Adjust elapsed timer to account for pause duration
            m_timer->start(1000 / m_targetFps);
            emit resumed();
        }
    }

signals:
    // Emitted for each fixed timestep update (for logic/physics)
    void fixedUpdate(double fixedDeltaTime);
    
    // Emitted each frame with render interpolation factor
    void frameUpdate(double deltaTime, double interpolation);
    
    // State changes
    void started();
    void stopped();
    void paused();
    void resumed();

private slots:
    void tick() {
        qint64 currentTime = m_elapsed.elapsed();
        m_actualDeltaTime = (currentTime - m_lastTime) / 1000.0;
        m_lastTime = currentTime;
        m_frameCount++;
        
        // Clamp delta time to prevent spiral of death
        double frameTime = qMin(m_actualDeltaTime, 0.25);
        
        // Apply speed multiplier to logical time
        double scaledDelta = frameTime * effectiveSpeed();
        m_accumulator += scaledDelta;
        
        // Fixed timestep updates
        int maxUpdates = 5; // Prevent too many updates in one frame
        int updateCount = 0;
        
        while (m_accumulator >= m_fixedDeltaTime && updateCount < maxUpdates) {
            emit fixedUpdate(m_fixedDeltaTime);
            m_accumulator -= m_fixedDeltaTime;
            updateCount++;
        }
        
        // Calculate interpolation for smooth rendering
        m_interpolation = m_accumulator / m_fixedDeltaTime;
        
        // Always emit frame update for rendering
        emit frameUpdate(m_actualDeltaTime, m_interpolation);
    }

private:
    QTimer* m_timer;
    QElapsedTimer m_elapsed;
    
    bool m_running;
    int m_targetFps;
    double m_fixedDeltaTime;
    
    qint64 m_frameCount = 0;
    qint64 m_lastTime = 0;
    qint64 m_pauseTime = 0;
    
    double m_accumulator;
    double m_interpolation;
    double m_actualDeltaTime = 0.0;
    
    double m_speedMultiplier = 1.0;
    Phase m_currentPhase = Phase::Normal;
};
