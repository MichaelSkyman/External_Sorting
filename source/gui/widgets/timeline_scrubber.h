#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QVector>
#include <QToolTip>

// Phase marker on timeline
struct PhaseMarker {
    int stepIndex;
    QString name;
    QColor color;
};

// Timeline scrubber for step-by-step navigation
class TimelineScrubber : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentStep READ currentStep WRITE setCurrentStep NOTIFY currentStepChanged)

public:
    explicit TimelineScrubber(QWidget* parent = nullptr);
    
    // Step management
    void setTotalSteps(int total);
    int totalSteps() const { return m_totalSteps; }
    
    int currentStep() const { return m_currentStep; }
    void setCurrentStep(int step);
    
    // Phase markers
    void addPhaseMarker(int stepIndex, const QString& name, const QColor& color);
    void clearPhaseMarkers();
    
    // Visual configuration
    void setTrackColor(const QColor& color) { m_trackColor = color; update(); }
    void setProgressColor(const QColor& color) { m_progressColor = color; update(); }
    void setHandleColor(const QColor& color) { m_handleColor = color; update(); }
    
    // Sizing
    QSize sizeHint() const override { return QSize(400, 50); }
    QSize minimumSizeHint() const override { return QSize(200, 40); }

signals:
    void currentStepChanged(int step);
    void seekRequested(int step);
    void scrubStarted();
    void scrubEnded();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    int stepFromPosition(int x) const;
    int positionFromStep(int step) const;
    QRectF trackRect() const;
    QRectF handleRect() const;
    void updateTooltip(const QPoint& pos);

    int m_totalSteps = 100;
    int m_currentStep = 0;
    bool m_dragging = false;
    bool m_hovering = false;
    int m_hoverStep = -1;
    
    // Visual styling
    QColor m_trackColor{50, 55, 60};
    QColor m_progressColor{0, 143, 150};
    QColor m_handleColor{0, 180, 190};
    QColor m_markerColor{255, 200, 100};
    
    // Phase markers
    QVector<PhaseMarker> m_markers;
    
    // Layout
    int m_trackHeight = 8;
    int m_handleWidth = 16;
    int m_handleHeight = 24;
    int m_padding = 20;
};
