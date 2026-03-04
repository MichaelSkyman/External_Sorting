#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <functional>

/**
 * @brief High-precision animation timer targeting a configurable frame rate.
 *
 * Wraps QTimer with Qt::PreciseTimer and emits a delta-time @c frame signal
 * each tick to drive smooth interpolation in the animation pipeline.
 */
class AnimationTimer : public QObject
{
    Q_OBJECT

public:
    explicit AnimationTimer(QObject* parent = nullptr)
        : QObject(parent)
        , frameTimer(new QTimer(this))
        , running(false)
        , targetFps(60)
    {
        frameTimer->setTimerType(Qt::PreciseTimer);
        connect(frameTimer, &QTimer::timeout, this, &AnimationTimer::onFrame);
    }
    
    void setTargetFps(int fps) {
        targetFps = qBound(1, fps, 144);
        if (running) {
            frameTimer->setInterval(1000 / targetFps);
        }
    }
    
    int getTargetFps() const { return targetFps; }
    
    bool isRunning() const { return running; }
    
    /// @brief Returns the time elapsed since the last frame in seconds.
    double getDeltaTime() const { return deltaTime; }

    /// @brief Returns the total time elapsed since start() was called, in seconds.
    double getElapsedTime() const {
        return elapsedTimer.elapsed() / 1000.0;
    }
    
    /// @brief Returns the total number of frames emitted since start().
    qint64 getFrameCount() const { return frameCount; }

public slots:
    void start() {
        if (!running) {
            running = true;
            frameCount = 0;
            lastFrameTime = 0;
            elapsedTimer.start();
            frameTimer->start(1000 / targetFps);
            emit started();
        }
    }
    
    void stop() {
        if (running) {
            running = false;
            frameTimer->stop();
            emit stopped();
        }
    }
    
    void pause() {
        if (running) {
            frameTimer->stop();
            emit paused();
        }
    }
    
    void resume() {
        if (running) {
            frameTimer->start(1000 / targetFps);
            emit resumed();
        }
    }

signals:
    void frame(double deltaTime); ///< Emitted each frame with the delta time in seconds.
    void started();               ///< Emitted when the timer starts.
    void stopped();               ///< Emitted when the timer stops.
    void paused();                ///< Emitted when the timer is paused.
    void resumed();               ///< Emitted when the timer is resumed.

private slots:
    void onFrame() {
        qint64 currentTime = elapsedTimer.elapsed();
        deltaTime = (currentTime - lastFrameTime) / 1000.0;
        lastFrameTime = currentTime;
        frameCount++;
        
        emit frame(deltaTime);
    }

private:
    QTimer* frameTimer;
    QElapsedTimer elapsedTimer;
    
    bool running;
    int targetFps;
    double deltaTime = 0.0;
    qint64 lastFrameTime = 0;
    qint64 frameCount = 0;
};
