#pragma once

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QElapsedTimer>

// Disk I/O operation type
enum class DiskIOType {
    Read,
    Write
};

// Represents an ongoing disk I/O operation
struct DiskIOOperation {
    DiskIOType type = DiskIOType::Read;
    int blockIndex = 0;
    int runIndex = -1;
    QString label;
    qreal progress = 0.0;
    QPointF sourcePos;
    QPointF targetPos;
    QColor color;
    bool active = false;
};

// Animated disk I/O simulation with visual feedback
class DiskIOAnimator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal transferProgress READ transferProgress WRITE setTransferProgress)
    Q_PROPERTY(qreal pulsePhase READ pulsePhase WRITE setPulsePhase)

public:
    explicit DiskIOAnimator(QWidget* parent = nullptr);
    
    // Latency configuration (milliseconds)
    void setReadLatency(int ms) { m_readLatency = ms; }
    void setWriteLatency(int ms) { m_writeLatency = ms; }
    int readLatency() const { return m_readLatency; }
    int writeLatency() const { return m_writeLatency; }
    
    // Start animated operations
    void animateRead(int blockIndex, const QString& label = "");
    void animateWrite(int runIndex, const QString& label = "");
    void animateTransfer(QPointF source, QPointF target, DiskIOType type, const QString& label = "");
    
    // Cancel/complete
    void cancelAll();
    void completeCurrentOperation();
    
    // State
    bool isOperationActive() const { return m_currentOp.active; }
    DiskIOType currentOperationType() const { return m_currentOp.type; }
    
    // Animation properties
    qreal transferProgress() const { return m_transferProgress; }
    void setTransferProgress(qreal progress);
    
    qreal pulsePhase() const { return m_pulsePhase; }
    void setPulsePhase(qreal phase);
    
    // Visual configuration
    void setDiskPosition(QPointF pos) { m_diskPos = pos; update(); }
    void setRamPosition(QPointF pos) { m_ramPos = pos; update(); }
    void setRunsPosition(QPointF pos) { m_runsPos = pos; update(); }
    
    // Sizing
    QSize sizeHint() const override { return QSize(600, 80); }
    QSize minimumSizeHint() const override { return QSize(300, 60); }

signals:
    void operationStarted(DiskIOType type, int index);
    void operationProgress(DiskIOType type, qreal progress);
    void operationCompleted(DiskIOType type, int index);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void startOperation(const DiskIOOperation& op);
    void drawTransferLine(QPainter& painter);
    void drawProgressIndicator(QPainter& painter);
    void drawStatusLabel(QPainter& painter);
    void drawDataPacket(QPainter& painter);
    QColor getTypeColor(DiskIOType type) const;
    
    // Current operation
    DiskIOOperation m_currentOp;
    
    // Latency settings
    int m_readLatency = 200;
    int m_writeLatency = 150;
    
    // Positions
    QPointF m_diskPos{100, 40};
    QPointF m_ramPos{300, 40};
    QPointF m_runsPos{500, 40};
    
    // Animation
    qreal m_transferProgress = 0.0;
    qreal m_pulsePhase = 0.0;
    
    QPropertyAnimation* m_transferAnim = nullptr;
    QPropertyAnimation* m_pulseAnim = nullptr;
    QTimer* m_pulseTimer = nullptr;
    
    // Visual
    int m_lineThickness = 4;
    int m_packetSize = 20;
};
