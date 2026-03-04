#include "timeline_scrubber.h"
#include <QPainterPath>
#include <QLinearGradient>
#include <QtMath>

TimelineScrubber::TimelineScrubber(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(40);
    setMaximumHeight(60);
}

void TimelineScrubber::setTotalSteps(int total)
{
    m_totalSteps = qMax(1, total);
    if (m_currentStep >= m_totalSteps) {
        setCurrentStep(m_totalSteps - 1);
    }
    update();
}

void TimelineScrubber::setCurrentStep(int step)
{
    step = qBound(0, step, m_totalSteps - 1);
    if (m_currentStep != step) {
        m_currentStep = step;
        emit currentStepChanged(step);
        update();
    }
}

void TimelineScrubber::addPhaseMarker(int stepIndex, const QString& name, const QColor& color)
{
    PhaseMarker marker;
    marker.stepIndex = stepIndex;
    marker.name = name;
    marker.color = color;
    m_markers.append(marker);
    update();
}

void TimelineScrubber::clearPhaseMarkers()
{
    m_markers.clear();
    update();
}

QRectF TimelineScrubber::trackRect() const
{
    qreal y = (height() - m_trackHeight) / 2.0;
    return QRectF(m_padding, y, width() - 2 * m_padding, m_trackHeight);
}

QRectF TimelineScrubber::handleRect() const
{
    int x = positionFromStep(m_currentStep);
    qreal y = (height() - m_handleHeight) / 2.0;
    return QRectF(x - m_handleWidth / 2.0, y, m_handleWidth, m_handleHeight);
}

int TimelineScrubber::stepFromPosition(int x) const
{
    QRectF track = trackRect();
    if (track.width() <= 0) return 0;
    
    qreal ratio = (x - track.left()) / track.width();
    ratio = qBound(0.0, ratio, 1.0);
    return static_cast<int>(ratio * (m_totalSteps - 1));
}

int TimelineScrubber::positionFromStep(int step) const
{
    QRectF track = trackRect();
    if (m_totalSteps <= 1) return static_cast<int>(track.left());
    
    qreal ratio = static_cast<qreal>(step) / (m_totalSteps - 1);
    return static_cast<int>(track.left() + ratio * track.width());
}

void TimelineScrubber::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRectF track = trackRect();
    
    // Background
    painter.fillRect(rect(), QColor(30, 32, 36));
    
    // Track background
    painter.setBrush(m_trackColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(track, m_trackHeight / 2, m_trackHeight / 2);
    
    // Progress fill
    if (m_totalSteps > 1 && m_currentStep > 0) {
        qreal progressWidth = track.width() * m_currentStep / (m_totalSteps - 1);
        QRectF progressRect(track.left(), track.top(), progressWidth, track.height());
        
        QLinearGradient gradient(progressRect.topLeft(), progressRect.topRight());
        gradient.setColorAt(0, m_progressColor.darker(120));
        gradient.setColorAt(1, m_progressColor);
        
        painter.setBrush(gradient);
        painter.drawRoundedRect(progressRect, m_trackHeight / 2, m_trackHeight / 2);
    }
    
    // Phase markers
    for (const auto& marker : m_markers) {
        int x = positionFromStep(marker.stepIndex);
        
        // Marker line
        painter.setPen(QPen(marker.color, 2));
        painter.drawLine(x, track.top() - 8, x, track.bottom() + 8);
        
        // Marker dot
        painter.setBrush(marker.color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(x, track.top() - 12), 4, 4);
    }
    
    // Step ticks (every 10%)
    painter.setPen(QPen(QColor(80, 85, 90), 1));
    for (int i = 0; i <= 10; i++) {
        int step = (m_totalSteps - 1) * i / 10;
        int x = positionFromStep(step);
        painter.drawLine(x, track.bottom() + 2, x, track.bottom() + 6);
    }
    
    // Hover preview
    if (m_hovering && m_hoverStep >= 0 && !m_dragging) {
        int hoverX = positionFromStep(m_hoverStep);
        painter.setPen(QPen(QColor(255, 255, 255, 100), 2));
        painter.drawLine(hoverX, track.top() - 4, hoverX, track.bottom() + 4);
    }
    
    // Handle
    QRectF handle = handleRect();
    
    // Handle shadow
    QRectF shadowRect = handle.adjusted(2, 2, 2, 2);
    painter.setBrush(QColor(0, 0, 0, 60));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(shadowRect, 4, 4);
    
    // Handle body
    QLinearGradient handleGrad(handle.topLeft(), handle.bottomLeft());
    handleGrad.setColorAt(0, m_handleColor.lighter(120));
    handleGrad.setColorAt(0.5, m_handleColor);
    handleGrad.setColorAt(1, m_handleColor.darker(120));
    
    painter.setBrush(handleGrad);
    painter.setPen(QPen(m_handleColor.darker(150), 1));
    painter.drawRoundedRect(handle, 4, 4);
    
    // Handle grip lines
    painter.setPen(QPen(QColor(255, 255, 255, 150), 1));
    qreal cx = handle.center().x();
    qreal cy = handle.center().y();
    painter.drawLine(QPointF(cx - 3, cy - 4), QPointF(cx - 3, cy + 4));
    painter.drawLine(QPointF(cx, cy - 4), QPointF(cx, cy + 4));
    painter.drawLine(QPointF(cx + 3, cy - 4), QPointF(cx + 3, cy + 4));
    
    // Step counter
    painter.setPen(QColor(180, 185, 190));
    QFont font = painter.font();
    font.setPixelSize(10);
    painter.setFont(font);
    
    QString stepText = QString("Step %1 / %2").arg(m_currentStep + 1).arg(m_totalSteps);
    painter.drawText(QRectF(0, height() - 15, width(), 15), Qt::AlignCenter, stepText);
}

void TimelineScrubber::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QRectF handle = handleRect();
        
        if (handle.contains(event->position())) {
            m_dragging = true;
            emit scrubStarted();
        } else {
            // Click to seek
            int step = stepFromPosition(event->position().x());
            setCurrentStep(step);
            emit seekRequested(step);
            m_dragging = true;
            emit scrubStarted();
        }
        
        update();
    }
}

void TimelineScrubber::mouseMoveEvent(QMouseEvent* event)
{
    m_hovering = true;
    m_hoverStep = stepFromPosition(event->position().x());
    
    if (m_dragging) {
        int step = stepFromPosition(event->position().x());
        setCurrentStep(step);
        emit seekRequested(step);
    }
    
    updateTooltip(event->pos());
    update();
}

void TimelineScrubber::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        emit scrubEnded();
        update();
    }
}

void TimelineScrubber::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y() > 0 ? 1 : -1;
    
    // Ctrl+wheel for bigger jumps
    if (event->modifiers() & Qt::ControlModifier) {
        delta *= 10;
    }
    
    int newStep = qBound(0, m_currentStep + delta, m_totalSteps - 1);
    if (newStep != m_currentStep) {
        setCurrentStep(newStep);
        emit seekRequested(newStep);
    }
    
    event->accept();
}

void TimelineScrubber::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void TimelineScrubber::updateTooltip(const QPoint& pos)
{
    int step = stepFromPosition(pos.x());
    
    // Check if near a phase marker
    QString tooltipText = QString("Step %1").arg(step + 1);
    
    for (const auto& marker : m_markers) {
        if (qAbs(marker.stepIndex - step) < 3) {
            tooltipText = QString("%1\n%2").arg(marker.name).arg(tooltipText);
            break;
        }
    }
    
    QToolTip::showText(mapToGlobal(pos), tooltipText, this);
}
