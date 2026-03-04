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
class FullscreenVisualizer;  // New fullscreen visualizer

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // File selection
    void browseInput();
    void browseOutput();
    
    // Main actions
    void startSorting();
    void onChunkModeChanged();
    
    // Page navigation
    void showSetupPage();
    void showAnimationPage();
    void onBackClicked();
    
    // Playback controls
    void onPauseToggled(bool checked);
    void onStepForward();
    void onStepBackward();
    void onSpeedChanged(int value);
    void onQueueSizeChanged(int size);
    
    // Timeline
    void onTimelineSeek(int stepIndex);
    void onStepIndexChanged(int currentIndex, int totalSteps);
    
    // Zoom controls
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onZoomChanged(qreal factor);
    
    // Sorting callbacks
    void onSortingFinished();
    void onSortingError(const QString& error);
    
    // Animation callbacks
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
    
    // Animation system
    AnimationController* animController;
    SortVisualizer* visualizer;
    QGraphicsScene* scene;
    
    // New fullscreen visualization system
    FullscreenVisualizer* fullscreenVisualizer = nullptr;
    QStackedWidget* visualizerStack = nullptr;
    bool useFullscreenMode = true;  // Toggle between legacy and fullscreen
    
    // New visualization widgets
    TimelineScrubber* timelineScrubber;
    MergeHeapWidget* heapWidget;
    DiskIOAnimator* diskIOAnimator;
    
    // Toolbar and controls
    QToolBar* controlToolbar;
    QToolButton* playBtn;
    QToolButton* pauseBtn;
    QToolButton* stepBackBtn;
    QToolButton* stepFwdBtn;
    QSlider* speedSlider;
    QLabel* speedLabel;
    QToolButton* zoomInBtn;
    QToolButton* zoomOutBtn;
    QToolButton* zoomResetBtn;
    QLabel* zoomLabel;
    
    // Status widgets
    QLabel* phaseLabel;
    QLabel* stepCountLabel;
    QSplitter* mainSplitter;
    
    // State
    bool isSorting = false;
    QVector<double> sortData;
    QString currentPhase;
};
