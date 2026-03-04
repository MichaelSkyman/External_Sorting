#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <functional>

// High-precision animation timer for smooth 60fps rendering
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
    
    // Get time since last frame in seconds (for interpolation)
    double getDeltaTime() const { return deltaTime; }
    
    // Get elapsed time since start in seconds
    double getElapsedTime() const {
        return elapsedTimer.elapsed() / 1000.0;
    }
    
    // Get current frame number
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
    void frame(double deltaTime);  // Emitted each frame with delta time
    void started();
    void stopped();
    void paused();
    void resumed();

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
