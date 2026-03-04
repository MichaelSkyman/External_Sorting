#include "disk_io_animator.h"
#include <QPainterPath>
#include <QLinearGradient>
#include <QtMath>

DiskIOAnimator::DiskIOAnimator(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    // Transfer animation
    m_transferAnim = new QPropertyAnimation(this, "transferProgress", this);
    m_transferAnim->setEasingCurve(QEasingCurve::InOutQuad);
    
    connect(m_transferAnim, &QPropertyAnimation::finished, this, [this]() {
        emit operationCompleted(m_currentOp.type, 
            m_currentOp.type == DiskIOType::Read ? m_currentOp.blockIndex : m_currentOp.runIndex);
        m_currentOp.active = false;
        update();
    });
    
    // Pulse animation for glow effect
    m_pulseAnim = new QPropertyAnimation(this, "pulsePhase", this);
    m_pulseAnim->setLoopCount(-1);  // Infinite
    m_pulseAnim->setDuration(1000);
    m_pulseAnim->setStartValue(0.0);
    m_pulseAnim->setEndValue(2.0 * M_PI);
}

void DiskIOAnimator::animateRead(int blockIndex, const QString& label)
{
    DiskIOOperation op;
    op.type = DiskIOType::Read;
    op.blockIndex = blockIndex;
    op.label = label.isEmpty() ? QString("Reading block %1...").arg(blockIndex + 1) : label;
    op.sourcePos = m_diskPos;
    op.targetPos = m_ramPos;
    op.color = getTypeColor(DiskIOType::Read);
    
    startOperation(op);
}

void DiskIOAnimator::animateWrite(int runIndex, const QString& label)
{
    DiskIOOperation op;
    op.type = DiskIOType::Write;
    op.runIndex = runIndex;
    op.label = label.isEmpty() ? QString("Writing run %1...").arg(runIndex + 1) : label;
    op.sourcePos = m_ramPos;
    op.targetPos = m_runsPos;
    op.color = getTypeColor(DiskIOType::Write);
    
    startOperation(op);
}

void DiskIOAnimator::animateTransfer(QPointF source, QPointF target, DiskIOType type, const QString& label)
{
    DiskIOOperation op;
    op.type = type;
    op.label = label;
    op.sourcePos = source;
    op.targetPos = target;
    op.color = getTypeColor(type);
    
    startOperation(op);
}

void DiskIOAnimator::startOperation(const DiskIOOperation& op)
{
    m_currentOp = op;
    m_currentOp.active = true;
    m_transferProgress = 0.0;
    
    int latency = (op.type == DiskIOType::Read) ? m_readLatency : m_writeLatency;
    
    m_transferAnim->setDuration(latency);
    m_transferAnim->setStartValue(0.0);
    m_transferAnim->setEndValue(1.0);
    m_transferAnim->start();
    
    m_pulseAnim->start();
    
    emit operationStarted(op.type, op.type == DiskIOType::Read ? op.blockIndex : op.runIndex);
    update();
}

void DiskIOAnimator::cancelAll()
{
    m_transferAnim->stop();
    m_pulseAnim->stop();
    m_currentOp.active = false;
    update();
}

void DiskIOAnimator::completeCurrentOperation()
{
    if (m_currentOp.active) {
        m_transferAnim->stop();
        m_transferProgress = 1.0;
        emit operationCompleted(m_currentOp.type,
            m_currentOp.type == DiskIOType::Read ? m_currentOp.blockIndex : m_currentOp.runIndex);
        m_currentOp.active = false;
        m_pulseAnim->stop();
        update();
    }
}

void DiskIOAnimator::setTransferProgress(qreal progress)
{
    m_transferProgress = progress;
    m_currentOp.progress = progress;
    emit operationProgress(m_currentOp.type, progress);
    update();
}

void DiskIOAnimator::setPulsePhase(qreal phase)
{
    m_pulsePhase = phase;
    if (m_currentOp.active) {
        update();
    }
}

QColor DiskIOAnimator::getTypeColor(DiskIOType type) const
{
    return (type == DiskIOType::Read) ? QColor(70, 150, 220) : QColor(70, 200, 120);
}

void DiskIOAnimator::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(40, 42, 46));
    bg.setColorAt(1, QColor(32, 34, 38));
    painter.fillRect(rect(), bg);
    
    if (!m_currentOp.active) {
        // Idle state
        painter.setPen(QColor(80, 85, 90));
        painter.drawText(rect(), Qt::AlignCenter, "Disk I/O - Idle");
        return;
    }
    
    drawTransferLine(painter);
    drawDataPacket(painter);
    drawProgressIndicator(painter);
    drawStatusLabel(painter);
}

void DiskIOAnimator::drawTransferLine(QPainter& painter)
{
    QPointF start(50, height() / 2.0);
    QPointF end(width() - 50, height() / 2.0);
    
    // Track line (background)
    painter.setPen(QPen(QColor(60, 65, 70), m_lineThickness, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(start, end);
    
    // Glowing progress line
    qreal progressWidth = (end.x() - start.x()) * m_transferProgress;
    QPointF progressEnd(start.x() + progressWidth, start.y());
    
    // Pulse glow
    qreal glowIntensity = 0.5 + 0.5 * qSin(m_pulsePhase);
    QColor glowColor = m_currentOp.color;
    glowColor.setAlphaF(glowIntensity * 0.5);
    
    // Draw glow layers
    for (int i = 3; i > 0; i--) {
        painter.setPen(QPen(glowColor, m_lineThickness + i * 4, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(start, progressEnd);
    }
    
    // Solid progress line
    QLinearGradient lineGrad(start, progressEnd);
    lineGrad.setColorAt(0, m_currentOp.color.darker(120));
    lineGrad.setColorAt(1, m_currentOp.color);
    
    painter.setPen(QPen(QBrush(lineGrad), m_lineThickness, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(start, progressEnd);
    
    // End points
    painter.setBrush(QColor(50, 55, 60));
    painter.setPen(QPen(QColor(80, 85, 90), 2));
    
    // Source icon (disk or RAM)
    QRectF sourceRect(start.x() - 25, start.y() - 15, 30, 30);
    painter.drawRoundedRect(sourceRect, 4, 4);
    
    painter.setPen(m_currentOp.type == DiskIOType::Read ? QColor(150, 155, 160) : m_currentOp.color);
    QFont iconFont = painter.font();
    iconFont.setPixelSize(10);
    iconFont.setBold(true);
    painter.setFont(iconFont);
    painter.drawText(sourceRect, Qt::AlignCenter, 
                    m_currentOp.type == DiskIOType::Read ? "DISK" : "RAM");
    
    // Target icon
    QRectF targetRect(end.x() - 5, end.y() - 15, 30, 30);
    painter.setBrush(QColor(50, 55, 60));
    painter.setPen(QPen(QColor(80, 85, 90), 2));
    painter.drawRoundedRect(targetRect, 4, 4);
    
    painter.setPen(m_currentOp.type == DiskIOType::Read ? m_currentOp.color : QColor(150, 155, 160));
    painter.drawText(targetRect, Qt::AlignCenter,
                    m_currentOp.type == DiskIOType::Read ? "RAM" : "DISK");
}

void DiskIOAnimator::drawDataPacket(QPainter& painter)
{
    QPointF start(50, height() / 2.0);
    QPointF end(width() - 50, height() / 2.0);
    
    // Interpolate packet position
    qreal x = start.x() + (end.x() - start.x()) * m_transferProgress;
    QPointF packetPos(x, start.y());
    
    // Packet glow
    qreal glowIntensity = 0.7 + 0.3 * qSin(m_pulsePhase * 2);
    QRadialGradient glow(packetPos, m_packetSize * 2);
    QColor glowColor = m_currentOp.color;
    glowColor.setAlphaF(glowIntensity * 0.4);
    glow.setColorAt(0, glowColor);
    glow.setColorAt(1, Qt::transparent);
    
    painter.setBrush(glow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(packetPos, m_packetSize * 2, m_packetSize * 2);
    
    // Packet body
    QRadialGradient packetGrad(packetPos - QPointF(5, 5), m_packetSize);
    packetGrad.setColorAt(0, m_currentOp.color.lighter(130));
    packetGrad.setColorAt(0.7, m_currentOp.color);
    packetGrad.setColorAt(1, m_currentOp.color.darker(120));
    
    painter.setBrush(packetGrad);
    painter.setPen(QPen(m_currentOp.color.darker(150), 2));
    painter.drawEllipse(packetPos, m_packetSize, m_packetSize);
    
    // Arrow inside packet
    painter.setPen(QPen(Qt::white, 2));
    QPointF arrowStart = packetPos - QPointF(6, 0);
    QPointF arrowEnd = packetPos + QPointF(6, 0);
    painter.drawLine(arrowStart, arrowEnd);
    painter.drawLine(arrowEnd, arrowEnd + QPointF(-4, -4));
    painter.drawLine(arrowEnd, arrowEnd + QPointF(-4, 4));
}

void DiskIOAnimator::drawProgressIndicator(QPainter& painter)
{
    // Progress percentage
    int percent = static_cast<int>(m_transferProgress * 100);
    
    painter.setPen(m_currentOp.color);
    QFont font = painter.font();
    font.setPixelSize(12);
    font.setBold(true);
    painter.setFont(font);
    
    QString progressText = QString("%1%").arg(percent);
    painter.drawText(QRectF(width() / 2 - 30, height() - 20, 60, 20), 
                    Qt::AlignCenter, progressText);
}

void DiskIOAnimator::drawStatusLabel(QPainter& painter)
{
    painter.setPen(QColor(200, 205, 210));
    QFont font = painter.font();
    font.setPixelSize(11);
    font.setBold(false);
    painter.setFont(font);
    
    painter.drawText(QRectF(0, 5, width(), 20), Qt::AlignCenter, m_currentOp.label);
}
