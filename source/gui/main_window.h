#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QThread>
#include <QVector>
#include <QToolBar>
#include <QSplitter>
#include <QLabel>
#include <QSlider>
#include <QToolButton>
#include <QStackedWidget>
#include "../app/config.h"

// Forward declarations
class AnimationController;
class SortVisualizer;
class TimelineScrubber;
class MergeHeapWidget;
class DiskIOAnimator;
class FullscreenVisualizer;

namespace Ui { class MainWindow; }

/**
 * @brief Main application window for the External Sorting visualizer.
 *
 * Manages the setup UI, sorting configuration, and drives the animation
 * pipeline via AnimationController (legacy) and FullscreenVisualizer (primary).
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    /// @name File Selection
    void browseInput();
    void browseOutput();

    /// @name Main Actions
    void startSorting();
    void onChunkModeChanged();

    /// @name Page Navigation
    void showSetupPage();
    void showAnimationPage();
    void onBackClicked();

    /// @name Playback Controls
    void onPauseToggled(bool checked);
    void onStepForward();
    void onStepBackward();
    void onSpeedChanged(int value);
    void onQueueSizeChanged(int size);

    /// @name Timeline
    void onTimelineSeek(int stepIndex);
    void onStepIndexChanged(int currentIndex, int totalSteps);

    /// @name Zoom Controls
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onZoomChanged(qreal factor);

    /// @name Sorting Callbacks
    void onSortingFinished();
    void onSortingError(const QString& error);

    /// @name Animation Callbacks
    void onStepStarted(const struct AnimationStep& step);
    void onStepCompleted(const struct AnimationStep& step);
    void onPhaseChanged(const QString& phase);

private:
    void setSetupVisible(bool visible);
    void setAnimationVisible(bool visible);
    void setupConnections();
    void initializeVisualizer();
    void createToolbar();
    void createStatusWidgets();
    void generateAnimationSteps(const QVector<double>& data, int chunkSize);
    void generateFullscreenAnimationSteps(const QVector<double>& data, int chunkSize);
    void updatePhaseIndicator(const QString& phase);

    Ui::MainWindow* ui;
    AppConfig config;
    
    AnimationController* animController; ///< Legacy step-based animation controller.
    SortVisualizer*      visualizer;     ///< Legacy QWidget-based sort visualizer.
    QGraphicsScene*      scene;          ///< Graphics scene used by the legacy visualizer.

    FullscreenVisualizer* fullscreenVisualizer = nullptr; ///< Primary fullscreen visualizer.
    QStackedWidget*       visualizerStack      = nullptr; ///< Stacks legacy and fullscreen views.
    bool useFullscreenMode = true; ///< When true, the fullscreen visualizer is active.

    TimelineScrubber* timelineScrubber; ///< Scrubber widget for step-by-step navigation.
    MergeHeapWidget*  heapWidget;       ///< Widget visualizing the k-way merge heap.
    DiskIOAnimator*   diskIOAnimator;   ///< Widget animating disk read/write transfers.

    QToolBar*    controlToolbar; ///< Toolbar housing playback and zoom controls.
    QToolButton* playBtn;        ///< Play button.
    QToolButton* pauseBtn;       ///< Pause/resume toggle button.
    QToolButton* stepBackBtn;    ///< Step-backward button.
    QToolButton* stepFwdBtn;     ///< Step-forward button.
    QSlider*     speedSlider;    ///< Speed control slider.
    QLabel*      speedLabel;     ///< Label showing the current speed multiplier.
    QToolButton* zoomInBtn;      ///< Zoom-in button.
    QToolButton* zoomOutBtn;     ///< Zoom-out button.
    QToolButton* zoomResetBtn;   ///< Zoom reset button.
    QLabel*      zoomLabel;      ///< Label showing the current zoom level.

    QLabel*    phaseLabel;     ///< Label showing the current sort phase.
    QLabel*    stepCountLabel; ///< Label showing current/total step count.
    QSplitter* mainSplitter;   ///< Splitter dividing the setup and visualizer areas.

    bool            isSorting   = false; ///< True while sorting animation is in progress.
    QVector<double> sortData;            ///< Loaded input data for the current session.
    QString         currentPhase;        ///< Name of the currently active sort phase.
};
