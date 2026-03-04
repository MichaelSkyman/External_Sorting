#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QVector>
#include <QToolTip>

/** @brief Marks the start of a sort phase on the timeline track. */
struct PhaseMarker {
    int stepIndex;
    QString name;
    QColor color;
};

/** @brief Interactive timeline scrubber for step-by-step animation navigation. */
class TimelineScrubber : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentStep READ currentStep WRITE setCurrentStep NOTIFY currentStepChanged)

public:
    explicit TimelineScrubber(QWidget* parent = nullptr);
    
    /// @brief Sets the total number of animation steps on the timeline.
    void setTotalSteps(int total);
    int totalSteps() const { return m_totalSteps; } ///< Returns the total step count.

    int  currentStep() const { return m_currentStep; } ///< Returns the current step index.
    /// @brief Moves the scrubber handle to @p step.
    void setCurrentStep(int step);

    /// @brief Adds a named, colored phase marker at @p stepIndex.
    void addPhaseMarker(int stepIndex, const QString& name, const QColor& color);
    /// @brief Removes all phase markers from the timeline.
    void clearPhaseMarkers();

    /// @brief Sets the track background color.
    void setTrackColor(const QColor& color)    { m_trackColor    = color; update(); }
    /// @brief Sets the progress fill color.
    void setProgressColor(const QColor& color) { m_progressColor = color; update(); }
    /// @brief Sets the scrubber handle color.
    void setHandleColor(const QColor& color)   { m_handleColor   = color; update(); }
    
    /// @brief Returns the recommended size for the scrubber widget.
    QSize sizeHint() const override { return QSize(400, 50); }
    /// @brief Returns the minimum acceptable size for the scrubber widget.
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

    int  m_totalSteps  = 100;  ///< Total number of steps on the timeline.
    int  m_currentStep = 0;    ///< Current step index (position of the handle).
    bool m_dragging    = false; ///< True while the user is dragging the handle.
    bool m_hovering    = false; ///< True while the cursor is over the track.
    int  m_hoverStep   = -1;   ///< Step index under the cursor (-1 = none).

    QColor m_trackColor   {50, 55, 60};    ///< Background track color.
    QColor m_progressColor{0, 143, 150};   ///< Filled progress color.
    QColor m_handleColor  {0, 180, 190};   ///< Scrubber handle color.
    QColor m_markerColor  {255, 200, 100}; ///< Phase marker color.

    QVector<PhaseMarker> m_markers; ///< Phase markers displayed on the track.

    int m_trackHeight  = 8;  ///< Height of the track bar in pixels.
    int m_handleWidth  = 16; ///< Width of the scrubber handle in pixels.
    int m_handleHeight = 24; ///< Height of the scrubber handle in pixels.
    int m_padding      = 20; ///< Horizontal padding on each side of the track.
};
