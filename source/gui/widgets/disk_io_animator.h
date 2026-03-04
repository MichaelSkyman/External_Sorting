#pragma once

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QElapsedTimer>

/** @brief Direction of a disk I/O operation. */
enum class DiskIOType {
    Read,  ///< Data is being read from disk into RAM.
    Write  ///< Data is being written from RAM to disk.
};

/** @brief Describes an in-progress disk I/O operation for animation. */
struct DiskIOOperation {
    DiskIOType type       = DiskIOType::Read; ///< Read or Write direction.
    int        blockIndex = 0;                ///< Index of the block being transferred.
    int        runIndex   = -1;               ///< Index of the target run file (-1 = none).
    QString    label;                         ///< Optional display label.
    qreal      progress   = 0.0;              ///< Transfer progress (0–1).
    QPointF    sourcePos;                     ///< Visual source position.
    QPointF    targetPos;                     ///< Visual target position.
    QColor     color;                         ///< Color representing the run or block type.
    bool       active     = false;            ///< True while this operation is in progress.
};

/** @brief Animated widget simulating disk read/write transfers with visual feedback. */
class DiskIOAnimator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal transferProgress READ transferProgress WRITE setTransferProgress)
    Q_PROPERTY(qreal pulsePhase READ pulsePhase WRITE setPulsePhase)

public:
    explicit DiskIOAnimator(QWidget* parent = nullptr);
    
    /// @brief Sets the simulated read latency in milliseconds.
    void setReadLatency(int ms)  { m_readLatency  = ms; }
    /// @brief Sets the simulated write latency in milliseconds.
    void setWriteLatency(int ms) { m_writeLatency = ms; }
    int readLatency()  const { return m_readLatency; }  ///< Returns the simulated read latency (ms).
    int writeLatency() const { return m_writeLatency; } ///< Returns the simulated write latency (ms).

    /// @brief Starts an animated read transfer for @p blockIndex.
    void animateRead(int blockIndex, const QString& label = "");
    /// @brief Starts an animated write transfer for @p runIndex.
    void animateWrite(int runIndex, const QString& label = "");
    /// @brief Starts a custom animated transfer from @p source to @p target.
    void animateTransfer(QPointF source, QPointF target, DiskIOType type, const QString& label = "");

    /// @brief Cancels all active operations and resets the animator.
    void cancelAll();
    /// @brief Marks the current operation as complete.
    void completeCurrentOperation();

    bool       isOperationActive()    const { return m_currentOp.active; } ///< True if an operation is in progress.
    DiskIOType currentOperationType() const { return m_currentOp.type; }   ///< Returns the current operation type.

    qreal transferProgress() const { return m_transferProgress; } ///< Transfer progress (0–1).
    /// @brief Sets the transfer animation progress property.
    void setTransferProgress(qreal progress);

    qreal pulsePhase() const { return m_pulsePhase; } ///< Current pulse animation phase.
    /// @brief Sets the pulse animation phase property.
    void setPulsePhase(qreal phase);

    /// @brief Sets the visual position of the disk indicator.
    void setDiskPosition(QPointF pos) { m_diskPos  = pos; update(); }
    /// @brief Sets the visual position of the RAM indicator.
    void setRamPosition(QPointF pos)  { m_ramPos   = pos; update(); }
    /// @brief Sets the visual position of the runs indicator.
    void setRunsPosition(QPointF pos) { m_runsPos  = pos; update(); }
    
    /// @brief Returns the recommended size for the disk I/O animator widget.
    QSize sizeHint() const override { return QSize(600, 80); }
    /// @brief Returns the minimum acceptable size for the disk I/O animator widget.
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
    
    DiskIOOperation m_currentOp; ///< The currently active I/O operation.

    int m_readLatency  = 200; ///< Simulated read latency in milliseconds.
    int m_writeLatency = 150; ///< Simulated write latency in milliseconds.

    QPointF m_diskPos {100, 40}; ///< Visual position of the disk icon.
    QPointF m_ramPos  {300, 40}; ///< Visual position of the RAM icon.
    QPointF m_runsPos{500, 40}; ///< Visual position of the runs icon.

    qreal m_transferProgress = 0.0; ///< Transfer animation progress (0–1).
    qreal m_pulsePhase       = 0.0; ///< Current phase of the pulse animation.

    QPropertyAnimation* m_transferAnim = nullptr; ///< Animation driving transfer progress.
    QPropertyAnimation* m_pulseAnim    = nullptr; ///< Animation driving pulse phase.
    QTimer*             m_pulseTimer   = nullptr; ///< Timer triggering pulse restarts.

    int m_lineThickness = 4;  ///< Thickness of the transfer line in pixels.
    int m_packetSize    = 20; ///< Size of the animated data-packet square in pixels.
};
