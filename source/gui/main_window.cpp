#include "main_window.h"
#include "ui_mainwindow.h"
#include "animation/animation_controller.h"
#include "animation/animation_step.h"
#include "animation/step_aggregator.h"
#include "visualization/sort_visualizer.h"
#include "visualization/fullscreen_visualizer.h"
#include "widgets/timeline_scrubber.h"
#include "widgets/merge_heap_widget.h"
#include "widgets/disk_io_animator.h"
#include "binary_stream_writer.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QDataStream>
#include <QFrame>
#include <QStackedWidget>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <limits>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , animController(nullptr)
    , visualizer(nullptr)
    , scene(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("External Sorting Demo");
    
    // Initialize visualization system
    initializeVisualizer();
    
    // Setup connections
    setupConnections();
    
    // Radio button group
    QButtonGroup* modeGroup = new QButtonGroup(this);
    modeGroup->addButton(ui->autoRadio);
    modeGroup->addButton(ui->manualRadio);
    
    // Initialize speed to slow
    ui->speedSlider->setValue(1);
    onSpeedChanged(1);
    
    // Start with setup page
    showSetupPage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeVisualizer()
{
    // Create animation controller
    animController = new AnimationController(this);
    
    // Enable adaptive speed for educational pacing
    animController->setAdaptiveSpeed(true);
    
    // === FULLSCREEN VISUALIZER (NEW - recommended) ===
    fullscreenVisualizer = new FullscreenVisualizer(this);
    connect(fullscreenVisualizer, &FullscreenVisualizer::backRequested,
            this, &MainWindow::onBackClicked);
    connect(fullscreenVisualizer, &FullscreenVisualizer::stepChanged,
            this, [this](int current, int total) {
                // Sync progress bar
                if (total > 0) {
                    ui->progressBar->setValue((current * 100) / total);
                }
            });
    
    // === LEGACY VISUALIZER ===
    // Create main visualizer widget - DOMINANT element
    visualizer = new SortVisualizer(this);
    visualizer->setController(animController);
    visualizer->setMinimumHeight(450);  // Increased from 400
    visualizer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Create compact timeline scrubber
    timelineScrubber = new TimelineScrubber(this);
    timelineScrubber->setFixedHeight(45);  // Slightly smaller
    
    // Create merge heap widget (shown during merge phase)
    heapWidget = new MergeHeapWidget(this);
    heapWidget->setFixedWidth(220);  // Slightly narrower
    heapWidget->setMinimumHeight(180);
    heapWidget->hide(); // Initially hidden
    
    // Create compact disk I/O animator
    diskIOAnimator = new DiskIOAnimator(this);
    diskIOAnimator->setFixedHeight(50);  // More compact
    
    // Create compact toolbar
    createToolbar();
    
    // Create compact status widgets
    createStatusWidgets();
    
    // Find the graphics view in layout and replace with visualization stack
    QVBoxLayout* sortingLayout = qobject_cast<QVBoxLayout*>(
        ui->graphicsView->parentWidget()->layout());
    
    if (sortingLayout) {
        int idx = sortingLayout->indexOf(ui->graphicsView);
        if (idx >= 0) {
            sortingLayout->removeWidget(ui->graphicsView);
            ui->graphicsView->hide();
            
            // Create stacked widget to switch between visualizers
            visualizerStack = new QStackedWidget(this);
            
            // === INDEX 0: Fullscreen visualizer (default) ===
            visualizerStack->addWidget(fullscreenVisualizer);
            
            // === INDEX 1: Legacy visualizer with splitter ===
            QWidget* legacyContainer = new QWidget(this);
            QVBoxLayout* legacyLayout = new QVBoxLayout(legacyContainer);
            legacyLayout->setContentsMargins(0, 0, 0, 0);
            legacyLayout->setSpacing(2);
            
            // Create main visualization area with splitter
            mainSplitter = new QSplitter(Qt::Horizontal, legacyContainer);
            mainSplitter->setHandleWidth(3);  // Thinner handle
            
            // Left side: main visualization (takes 80% width when heap visible)
            QWidget* leftWidget = new QWidget(mainSplitter);
            QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
            leftLayout->setContentsMargins(0, 0, 0, 0);
            leftLayout->setSpacing(2);  // Minimal spacing
            
            // Compact phase indicator at top
            phaseLabel->setFixedHeight(30);  // Compact height
            leftLayout->addWidget(phaseLabel);
            
            // Main visualization (DOMINANT area - takes most space)
            leftLayout->addWidget(visualizer, 10);  // High stretch factor
            
            // Compact controls row: disk I/O + timeline
            QHBoxLayout* controlsRow = new QHBoxLayout();
            controlsRow->setContentsMargins(0, 0, 0, 0);
            controlsRow->setSpacing(4);
            controlsRow->addWidget(diskIOAnimator, 1);
            controlsRow->addWidget(timelineScrubber, 2);  // Timeline gets more space
            
            QWidget* controlsWidget = new QWidget(leftWidget);
            controlsWidget->setLayout(controlsRow);
            controlsWidget->setFixedHeight(55);
            leftLayout->addWidget(controlsWidget);
            
            leftWidget->setLayout(leftLayout);
            
            // Right side: heap widget (optional, collapses when hidden)
            QWidget* rightWidget = new QWidget(mainSplitter);
            QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
            rightLayout->setContentsMargins(2, 0, 0, 0);
            rightLayout->setSpacing(2);
            
            // Compact heap label
            QLabel* heapLabel = new QLabel("Merge Heap", rightWidget);
            heapLabel->setStyleSheet(
                "QLabel { font-size: 11px; font-weight: bold; color: #ff9800; padding: 4px; }"
            );
            rightLayout->addWidget(heapLabel);
            rightLayout->addWidget(heapWidget, 1);
            rightWidget->setLayout(rightLayout);
            
            mainSplitter->addWidget(leftWidget);
            mainSplitter->addWidget(rightWidget);
            mainSplitter->setSizes({800, 200});  // 80% / 20% split
            mainSplitter->setStretchFactor(0, 4);
            mainSplitter->setStretchFactor(1, 1);
            
            // Collapse right widget initially
            mainSplitter->widget(1)->hide();
            
            legacyLayout->addWidget(mainSplitter);
            visualizerStack->addWidget(legacyContainer);
            
            // Use fullscreen mode by default
            visualizerStack->setCurrentIndex(useFullscreenMode ? 0 : 1);

            // SIZE POLICY: visualizerStack MUST expand to fill all space
            visualizerStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            visualizerStack->setMinimumSize(0, 0); // no artificial floor

            sortingLayout->insertWidget(idx, visualizerStack, 1); // stretch = 1

            // Remove the trailing vertical spacer that steals space
            for (int si = sortingLayout->count() - 1; si >= 0; --si) {
                QLayoutItem* item = sortingLayout->itemAt(si);
                if (item && item->spacerItem()) {
                    sortingLayout->removeItem(item);
                    delete item;
                    break;          // only one spacer
                }
            }
        }
    }
    
    // Connect controller signals
    connect(animController, &AnimationController::stepStarted,
            this, &MainWindow::onStepStarted);
    connect(animController, &AnimationController::stepCompleted,
            this, &MainWindow::onStepCompleted);
    connect(animController, &AnimationController::queueSizeChanged,
            this, &MainWindow::onQueueSizeChanged);
    connect(animController, &AnimationController::queueEmpty,
            this, &MainWindow::onSortingFinished);
    connect(animController, &AnimationController::stepIndexChanged,
            this, &MainWindow::onStepIndexChanged);
    connect(animController, &AnimationController::totalStepsChanged,
            timelineScrubber, &TimelineScrubber::setTotalSteps);
    
    // Connect timeline
    connect(timelineScrubber, &TimelineScrubber::seekRequested,
            this, &MainWindow::onTimelineSeek);
    connect(timelineScrubber, &TimelineScrubber::seekRequested,
            animController, &AnimationController::seekToStep);
    
    // Connect zoom signals
    connect(visualizer, &SortVisualizer::zoomChanged,
            this, &MainWindow::onZoomChanged);
}

void MainWindow::createToolbar()
{
    controlToolbar = new QToolBar("Playback Controls", this);
    controlToolbar->setMovable(false);
    controlToolbar->setIconSize(QSize(24, 24));
    
    // Step backward
    stepBackBtn = new QToolButton(controlToolbar);
    stepBackBtn->setText("\u23EE");  // ⏮
    stepBackBtn->setToolTip("Step Backward");
    stepBackBtn->setEnabled(false);
    controlToolbar->addWidget(stepBackBtn);
    
    // Play button
    playBtn = new QToolButton(controlToolbar);
    playBtn->setText("\u25B6");  // ▶
    playBtn->setToolTip("Play");
    playBtn->setCheckable(true);
    controlToolbar->addWidget(playBtn);
    
    // Pause button
    pauseBtn = new QToolButton(controlToolbar);
    pauseBtn->setText("\u23F8");  // ⏸
    pauseBtn->setToolTip("Pause");
    pauseBtn->setCheckable(true);
    controlToolbar->addWidget(pauseBtn);
    
    // Step forward
    stepFwdBtn = new QToolButton(controlToolbar);
    stepFwdBtn->setText("\u23ED");  // ⏭
    stepFwdBtn->setToolTip("Step Forward");
    controlToolbar->addWidget(stepFwdBtn);
    
    controlToolbar->addSeparator();
    
    // Speed control
    controlToolbar->addWidget(new QLabel(" Speed: ", controlToolbar));
    speedSlider = new QSlider(Qt::Horizontal, controlToolbar);
    speedSlider->setRange(1, 10);
    speedSlider->setValue(3);
    speedSlider->setFixedWidth(100);
    controlToolbar->addWidget(speedSlider);
    speedLabel = new QLabel("1.0x", controlToolbar);
    speedLabel->setFixedWidth(40);
    controlToolbar->addWidget(speedLabel);
    
    controlToolbar->addSeparator();
    
    // Zoom controls
    zoomOutBtn = new QToolButton(controlToolbar);
    zoomOutBtn->setText("-");
    zoomOutBtn->setToolTip("Zoom Out");
    controlToolbar->addWidget(zoomOutBtn);
    
    zoomLabel = new QLabel("100%", controlToolbar);
    zoomLabel->setFixedWidth(50);
    zoomLabel->setAlignment(Qt::AlignCenter);
    controlToolbar->addWidget(zoomLabel);
    
    zoomInBtn = new QToolButton(controlToolbar);
    zoomInBtn->setText("+");
    zoomInBtn->setToolTip("Zoom In");
    controlToolbar->addWidget(zoomInBtn);
    
    zoomResetBtn = new QToolButton(controlToolbar);
    zoomResetBtn->setText("Reset");
    zoomResetBtn->setToolTip("Reset Zoom");
    controlToolbar->addWidget(zoomResetBtn);
    
    controlToolbar->addSeparator();
    
    // Step counter
    stepCountLabel = new QLabel("Step: 0/0", controlToolbar);
    controlToolbar->addWidget(stepCountLabel);
    
    addToolBar(Qt::TopToolBarArea, controlToolbar);
    
    // Connect toolbar buttons
    connect(stepBackBtn, &QToolButton::clicked, this, &MainWindow::onStepBackward);
    connect(stepFwdBtn, &QToolButton::clicked, this, &MainWindow::onStepForward);
    connect(playBtn, &QToolButton::clicked, [this](bool checked) {
        if (checked) {
            animController->play();
            playBtn->setText("\u23F8");  // Show pause icon
        } else {
            animController->pause();
            playBtn->setText("\u25B6");  // Show play icon
        }
    });
    connect(pauseBtn, &QToolButton::clicked, [this](bool checked) {
        if (checked) {
            animController->pause();
        } else {
            animController->resume();
        }
    });
    connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    connect(zoomInBtn, &QToolButton::clicked, this, &MainWindow::onZoomIn);
    connect(zoomOutBtn, &QToolButton::clicked, this, &MainWindow::onZoomOut);
    connect(zoomResetBtn, &QToolButton::clicked, this, &MainWindow::onZoomReset);
}

void MainWindow::createStatusWidgets()
{
    // Compact phase indicator label
    phaseLabel = new QLabel("Ready", this);
    phaseLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  padding: 4px 12px;"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #3d4451, stop:1 #2d3441);"
        "  border-radius: 3px;"
        "  color: #00b4d8;"
        "}"
    );
    phaseLabel->setFixedHeight(30);
}

void MainWindow::setupConnections()
{
    connect(ui->browseInputBtn, &QPushButton::clicked,
            this, &MainWindow::browseInput);
    connect(ui->browseOutputBtn, &QPushButton::clicked,
            this, &MainWindow::browseOutput);
    connect(ui->startBtn, &QPushButton::clicked,
            this, &MainWindow::startSorting);
    connect(ui->autoRadio, &QRadioButton::toggled,
            this, &MainWindow::onChunkModeChanged);
    
    // Playback controls (old UI buttons - keep for compatibility)
    if (ui->pauseBtn) {
        connect(ui->pauseBtn, &QPushButton::toggled,
                this, &MainWindow::onPauseToggled);
    }
    if (ui->stepBtn) {
        connect(ui->stepBtn, &QPushButton::clicked,
                this, &MainWindow::onStepForward);
    }
    if (ui->speedSlider) {
        connect(ui->speedSlider, &QSlider::valueChanged,
                this, &MainWindow::onSpeedChanged);
    }
    connect(ui->backBtn, &QPushButton::clicked,
            this, &MainWindow::onBackClicked);
}

void MainWindow::showSetupPage()
{
    setSetupVisible(true);
    setAnimationVisible(false);
}

void MainWindow::showAnimationPage()
{
    setSetupVisible(false);
    setAnimationVisible(true);
}

void MainWindow::setSetupVisible(bool visible)
{
    ui->groupBox_files->setVisible(visible);
    ui->groupBox_chunk->setVisible(visible);
    ui->startBtn->setVisible(visible);
    if (ui->lab_sorting_title) ui->lab_sorting_title->setVisible(visible);
}

void MainWindow::setAnimationVisible(bool visible)
{
    // ================================================================
    // FULLSCREEN MODE: the canvas must occupy the ENTIRE window.
    // Hide title bar, sidebar, all non-essential UI chrome.
    // Zero-out all margins so visualizerStack expands edge-to-edge.
    // ================================================================
    if (useFullscreenMode && visible) {
        // --- Hide application chrome ---
        if (ui->frame_top)          ui->frame_top->setVisible(false);
        if (ui->frame_bottom_west)  ui->frame_bottom_west->setVisible(false);

        // --- Hide ALL sorting-page widgets except visualizerStack ---
        if (ui->lab_sorting_title)       ui->lab_sorting_title->setVisible(false);
        if (ui->visualizationLabel)      ui->visualizationLabel->setVisible(false);
        if (ui->playbackControlsFrame)   ui->playbackControlsFrame->setVisible(false);
        if (ui->groupBox_progress)       ui->groupBox_progress->setVisible(false);
        if (ui->backBtn)                 ui->backBtn->setVisible(false);
        if (mainSplitter)                mainSplitter->setVisible(false);
        if (timelineScrubber)            timelineScrubber->setVisible(false);
        if (diskIOAnimator)              diskIOAnimator->setVisible(false);
        if (phaseLabel)                  phaseLabel->setVisible(false);
        if (controlToolbar)              controlToolbar->setVisible(false);

        // --- Zero margins on every layout in the path ---
        if (auto* sl = qobject_cast<QVBoxLayout*>(ui->page_home->layout())) {
            sl->setContentsMargins(0, 0, 0, 0);
            sl->setSpacing(0);
        }
        if (ui->frame_bottom_east && ui->frame_bottom_east->layout()) {
            ui->frame_bottom_east->layout()->setContentsMargins(0, 0, 0, 0);
            ui->frame_bottom_east->layout()->setSpacing(0);
        }
        if (auto* frame = ui->frame_bottom) {
            if (frame->layout()) {
                frame->layout()->setContentsMargins(0, 0, 0, 0);
                frame->layout()->setSpacing(0);
            }
        }

        // --- Show only the canvas stack ---
        if (visualizerStack) {
            visualizerStack->setVisible(true);
            visualizerStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }

        qDebug() << "[Fullscreen] visualizerStack size:" << (visualizerStack ? visualizerStack->size() : QSize());
        return;
    }

    // ================================================================
    // RESTORE: bring back all chrome when leaving animation or legacy
    // ================================================================
    if (!visible) {
        // Restore chrome
        if (ui->frame_top)          ui->frame_top->setVisible(true);
        if (ui->frame_bottom_west)  ui->frame_bottom_west->setVisible(true);

        // Restore sorting-page margins
        if (auto* sl = qobject_cast<QVBoxLayout*>(ui->page_home->layout())) {
            sl->setContentsMargins(20, 20, 20, 20);
            sl->setSpacing(15);
        }
    }

    // Legacy mode or hiding animation
    if (ui->visualizationLabel)      ui->visualizationLabel->setVisible(visible);
    if (ui->playbackControlsFrame)   ui->playbackControlsFrame->setVisible(visible);

    if (mainSplitter)    mainSplitter->setVisible(visible);
    if (visualizer)      visualizer->setVisible(visible);
    if (timelineScrubber) timelineScrubber->setVisible(visible);
    if (diskIOAnimator)  diskIOAnimator->setVisible(visible);
    if (phaseLabel)      phaseLabel->setVisible(visible);
    if (controlToolbar)  controlToolbar->setVisible(visible);

    if (ui->groupBox_progress)  ui->groupBox_progress->setVisible(visible);
    if (ui->backBtn)             ui->backBtn->setVisible(visible);
    if (visualizerStack)         visualizerStack->setVisible(visible);
}

void MainWindow::onBackClicked()
{
    // Allow going back from fullscreen mode even during sorting
    if (useFullscreenMode && fullscreenVisualizer) {
        fullscreenVisualizer->stop();
    } else {
        animController->stop();
        animController->clearQueue();
        visualizer->clear();
    }

    isSorting = false;
    showSetupPage();
}

void MainWindow::onChunkModeChanged()
{
    bool isManual = ui->manualRadio->isChecked();
    ui->chunkSpinBox->setEnabled(isManual);
}

void MainWindow::browseInput()
{
    QString file = QFileDialog::getOpenFileName(
        this, "Select Input File", "", 
        "Binary Files (*.bin);;All Files (*)");
    
    if (!file.isEmpty()) {
        ui->inputEdit->setText(file);
        config.setInputFile(file.toStdString());
    }
}

void MainWindow::browseOutput()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Output Directory", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dir.isEmpty()) {
        ui->outputEdit->setText(dir);
        config.setOutputDir(dir.toStdString());
    }
}

void MainWindow::startSorting()
{
    QString inputFile = ui->inputEdit->text();
    QString outputDir  = ui->outputEdit->text();
    
    if (inputFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select an input file.");
        return;
    }
    if (outputDir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select an output directory.");
        return;
    }

    // Auto-generate output path: <dir>/output.bin (overwrite by default)
    std::string outputPath = BinaryStreamWriter::resolveOutputPath(
        outputDir.toStdString(), /*overwrite=*/true);
    
    // Read input file
    QFile file(inputFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", 
            "Cannot open input file: " + file.errorString());
        return;
    }
    
    sortData.clear();
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    in.setFloatingPointPrecision(QDataStream::DoublePrecision);
    
    while (!in.atEnd()) {
        double value;
        in >> value;
        sortData.append(value);
    }
    file.close();
    
    if (sortData.isEmpty()) {
        QMessageBox::warning(this, "Error", "Input file is empty or invalid.");
        return;
    }
    
    // Get chunk size
    int chunkSize = ui->autoRadio->isChecked() 
        ? qMax(10, sortData.size() / 4)  // Auto: ~4 chunks
        : ui->chunkSpinBox->value();
    
    // Transition to animation page
    showAnimationPage();
    
    // Initialize visualizer with data (both modes)
    if (useFullscreenMode && fullscreenVisualizer) {
        // Viewport model: canvas stores only metadata (total count, min, max)
        // No per-element block objects — step generation handles sampling
        fullscreenVisualizer->setData(sortData);
        visualizerStack->setCurrentIndex(0);

        // Generate viewport-friendly steps (internally samples for perf)
        generateFullscreenAnimationSteps(sortData, chunkSize);
        
        // Start playback
        isSorting = true;
        ui->statusLabel->setText("Sorting...");
        ui->progressBar->setValue(0);
        fullscreenVisualizer->play();
    } else {
        // Use legacy visualizer
        visualizer->setData(sortData);
        visualizerStack->setCurrentIndex(1);
        
        // Generate animation steps
        generateAnimationSteps(sortData, chunkSize);
        
        // Start playback
        isSorting = true;
        ui->statusLabel->setText("Sorting...");
        ui->progressBar->setValue(0);
        animController->play();
    }

    // ── Write sorted output as 32-bit signed int binary ──────────
    try {
        QVector<double> sorted = sortData;
        std::sort(sorted.begin(), sorted.end());

        BinaryStreamWriter writer(outputPath);
        for (double v : sorted)
            writer.writeFromDouble(v);
        writer.close();

        qDebug() << "Output written:" << QString::fromStdString(outputPath)
                 << "(" << sorted.size() << "values )";
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Output Error",
            QString("Could not write output file:\n%1").arg(e.what()));
    }
}

void MainWindow::generateAnimationSteps(const QVector<double>& data, int chunkSize)
{
    animController->clearQueue();
    
    // === INTRO SEQUENCE ===
    animController->enqueueIntroSequence(data);
    
    // === RUN GENERATION PHASE ===
    int numChunks = (data.size() + chunkSize - 1) / chunkSize;
    
    for (int c = 0; c < numChunks; c++) {
        int start = c * chunkSize;
        int end = qMin(start + chunkSize, data.size());
        
        // Extract chunk
        QVector<double> chunk;
        for (int i = start; i < end; i++) {
            chunk.append(data[i]);
        }
        
        // Load to RAM step
        auto loadStep = AnimationStep::createLoadToRAM(c, chunk);
        loadStep.durationMs = 1200;
        animController->enqueueStep(loadStep);
        
        // Pause to observe
        animController->enqueueStep(AnimationStep::createPause(800, "Block loaded into RAM"));
        
        // Sort transition
        animController->enqueueSortPhaseTransition();
        
        // Sort in RAM
        QVector<double> sortedChunk = chunk;
        std::sort(sortedChunk.begin(), sortedChunk.end());
        
        auto sortStep = AnimationStep::createSortBlock(c, sortedChunk);
        sortStep.durationMs = 1500;
        animController->enqueueStep(sortStep);
        
        // Pause after sorting
        animController->enqueueStep(AnimationStep::createPause(600, "Block sorted"));
        
        // Write run
        auto writeStep = AnimationStep::createWriteRun(c, sortedChunk);
        writeStep.durationMs = 1000;
        animController->enqueueStep(writeStep);
        
        // Progress update
        int progressPercent = ((c + 1) * 50) / numChunks;  // 0-50% for run generation
        AnimationStep progressStep;
        progressStep.type = StepType::StatusUpdate;
        progressStep.progress = progressPercent;
        progressStep.statusText = QString("Generated run %1 of %2").arg(c + 1).arg(numChunks);
        animController->enqueueStep(progressStep);
    }
    
    // === MERGE PHASE ===
    animController->enqueueMergePhaseTransition(numChunks);
    
    // Simulate merge (simplified - actual merge would come from algorithm)
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    
    // Add merge steps with progress
    for (int i = 0; i < sorted.size(); i++) {
        auto outputStep = AnimationStep::createWriteOutput(sorted[i], i);
        
        // Slow down early outputs for visibility
        if (i < 20) {
            outputStep.durationMs = 600;
        } else if (i < 50) {
            outputStep.durationMs = 300;
        } else {
            outputStep.durationMs = 100;
        }
        
        animController->enqueueStep(outputStep);
        
        // Periodic progress updates
        if (i % 10 == 0 || i == sorted.size() - 1) {
            int progressPercent = 50 + ((i + 1) * 50) / sorted.size();
            AnimationStep progressStep;
            progressStep.type = StepType::StatusUpdate;
            progressStep.progress = progressPercent;
            animController->enqueueStep(progressStep);
        }
    }
    
    // Final completion
    animController->enqueueStep(AnimationStep::createPhaseTransition("Sorting Complete!"));
    animController->enqueueStep(AnimationStep::createPause(2000, "All data sorted successfully"));
}

void MainWindow::generateFullscreenAnimationSteps(const QVector<double>& data, int chunkSize)
{
    StepAggregator* aggregator = fullscreenVisualizer->aggregator();
    aggregator->clear();

    // ── Visualization limits ──────────────────────────────────────
    const int MAX_VIS_CHUNKS        = 8;   // max runs to show
    const int MAX_ELEMENTS_PER_CHUNK = 80;  // max sampled elems per chunk

    int numChunks    = qMax(1, (data.size() + chunkSize - 1) / chunkSize);
    int visNumChunks = qMin(numChunks, MAX_VIS_CHUNKS);

    // ── Sample chunks for visualization ──────────────────────────
    QVector<QVector<double>> chunkSamples;
    for (int c = 0; c < visNumChunks; ++c) {
        int start     = static_cast<int>(static_cast<qint64>(c) * data.size() / visNumChunks);
        int end       = static_cast<int>(static_cast<qint64>(c + 1) * data.size() / visNumChunks);
        int actualLen = end - start;

        QVector<double> sample;
        if (actualLen <= MAX_ELEMENTS_PER_CHUNK) {
            for (int i = start; i < end; ++i) sample.append(data[i]);
        } else {
            double stride = double(actualLen) / MAX_ELEMENTS_PER_CHUNK;
            for (int i = 0; i < MAX_ELEMENTS_PER_CHUNK; ++i)
                sample.append(data[start + static_cast<int>(i * stride)]);
        }
        chunkSamples.append(sample);
    }

    // ── INTRO ─────────────────────────────────────────────────────
    RawOperation introOp;
    introOp.type     = RawOperation::Type::PhaseChange;
    introOp.metadata = QString("External Sorting: %1 elements").arg(data.size());
    aggregator->addOperation(introOp);

    // ── RUN GENERATION (via aggregator) ──────────────────────────
    {
        RawOperation op;
        op.type     = RawOperation::Type::PhaseChange;
        op.metadata = QString("Run Generation (0/%1)").arg(visNumChunks);
        aggregator->addOperation(op);
    }

    QVector<QVector<double>> allSortedRuns;

    for (int c = 0; c < visNumChunks; ++c) {
        const auto& chunk = chunkSamples[c];

        // Load ops
        for (int i = 0; i < chunk.size(); ++i) {
            RawOperation op;
            op.type       = RawOperation::Type::LoadToRAM;
            op.blockIndex = i;
            op.value      = chunk[i];
            aggregator->addOperation(op);
        }

        // Sort ops (limited)
        int sortOps = qMin(static_cast<int>(chunk.size() * std::log2(chunk.size() + 1)), 30);
        for (int i = 0; i < sortOps; ++i) {
            RawOperation op;
            op.type        = RawOperation::Type::Compare;
            op.blockIndex  = i % chunk.size();
            op.secondIndex = (i + 1) % chunk.size();
            aggregator->addOperation(op);
        }

        // Write run (sorted values)
        QVector<double> sorted = chunk;
        std::sort(sorted.begin(), sorted.end());
        for (int i = 0; i < sorted.size(); ++i) {
            RawOperation op;
            op.type       = RawOperation::Type::CreateRun;
            op.blockIndex = i;
            op.value      = sorted[i];
            aggregator->addOperation(op);
        }
        allSortedRuns.append(sorted);

        // Phase update
        RawOperation phaseOp;
        phaseOp.type     = RawOperation::Type::PhaseChange;
        phaseOp.metadata = QString("Run Generation (%1/%2)").arg(c + 1).arg(visNumChunks);
        aggregator->addOperation(phaseOp);

        QApplication::processEvents();
    }

    // Process run-generation raw ops into aggregated steps
    aggregator->process();
    QVector<AggregatedStep> allSteps = aggregator->takeAllSteps();

    // ── K-WAY MERGE (manual steps for proper run-head visualization) ─
    {
        AggregatedStep phase;
        phase.type       = AggregatedStep::Type::PhaseTransition;
        phase.statusText = QString("K-Way Merge (%1 runs)").arg(visNumChunks);
        phase.duration   = 2000;
        allSteps.append(phase);
    }

    // Simulate k-way merge with run pointers
    QVector<int> pointers(visNumChunks, 0);
    int totalMergeElements = 0;
    for (auto& r : allSortedRuns) totalMergeElements += r.size();

    int maxMergeSteps = 120;
    int mergeInterval = qMax(1, totalMergeElements / maxMergeSteps);

    for (int m = 0; m < totalMergeElements; ++m) {
        // Find minimum across run heads
        int    minRun = -1;
        double minVal = std::numeric_limits<double>::max();
        for (int r = 0; r < visNumChunks; ++r) {
            if (pointers[r] < allSortedRuns[r].size()
                && allSortedRuns[r][pointers[r]] < minVal) {
                minVal = allSortedRuns[r][pointers[r]];
                minRun = r;
            }
        }
        if (minRun < 0) break;
        pointers[minRun]++;

        // Emit merge step periodically
        bool emit_step = (m % mergeInterval == 0) || (m < 15)
                         || (m == totalMergeElements - 1);
        if (emit_step) {
            AggregatedStep step;
            step.type        = AggregatedStep::Type::MergeStep;
            step.mergeMinRun = minRun;
            step.values      = { minVal };

            // Active run indices (non-exhausted)
            for (int r = 0; r < visNumChunks; ++r)
                if (pointers[r] < allSortedRuns[r].size()) step.sourceBlocks.append(r);
            if (!step.sourceBlocks.contains(minRun)) step.sourceBlocks.append(minRun);

            step.statusText     = QString("Merge: %1 from Run %2")
                                      .arg(minVal, 0, 'f', 0).arg(minRun);
            step.duration       = 500;
            step.operationCount = 1;
            allSteps.append(step);
        }
    }

    // ── COMPLETE ──────────────────────────────────────────────────
    {
        AggregatedStep step;
        step.type       = AggregatedStep::Type::Complete;
        step.statusText = "Sorting Complete!";
        step.duration   = 2000;
        allSteps.append(step);
    }

    // Enqueue everything
    AnimationEngine* engine = fullscreenVisualizer->engine();
    engine->clearQueue();
    engine->enqueueSteps(allSteps);
}

void MainWindow::onStepStarted(const AnimationStep& step)
{
    // Update status label
    if (!step.statusText.isEmpty()) {
        if (ui->statusLabel) ui->statusLabel->setText(step.statusText);
    }
    
    // Update progress bar
    if (step.progress > 0) {
        if (ui->progressBar) ui->progressBar->setValue(step.progress);
    }
    
    // Handle phase transitions
    if (step.type == StepType::PhaseTransition) {
        onPhaseChanged(step.statusText);
        
        // Add phase marker to timeline
        if (timelineScrubber) {
            QColor phaseColor(0, 180, 216);  // Default cyan
            if (step.statusText.contains("Run", Qt::CaseInsensitive)) {
                phaseColor = QColor(76, 175, 80);  // Green
            } else if (step.statusText.contains("Merge", Qt::CaseInsensitive)) {
                phaseColor = QColor(255, 152, 0);  // Orange
            }
            timelineScrubber->addPhaseMarker(
                animController->getCurrentStepIndex(), 
                step.statusText,
                phaseColor
            );
        }
    }
    
    // Update disk I/O animator based on step type
    if (step.type == StepType::LoadToRAM && diskIOAnimator) {
        // Disk read animation
        diskIOAnimator->animateRead(0.5);  // 0.5s animation
    }
    else if ((step.type == StepType::WriteToRun || step.type == StepType::WriteOutput) 
             && diskIOAnimator) {
        // Disk write animation
        diskIOAnimator->animateWrite(0.5);
    }
}

void MainWindow::onStepCompleted(const AnimationStep& step)
{
    // Update progress if specified
    if (step.progress > 0) {
        ui->progressBar->setValue(step.progress);
    }
}

void MainWindow::onSortingFinished()
{
    isSorting = false;
    ui->statusLabel->setText("Sorting complete!");
    ui->progressBar->setValue(100);
    ui->pauseBtn->setChecked(false);
}

void MainWindow::onSortingError(const QString& error)
{
    isSorting = false;
    QMessageBox::critical(this, "Sorting Error", error);
    ui->statusLabel->setText("Error: " + error);
    showSetupPage();
}

void MainWindow::onPauseToggled(bool checked)
{
    if (checked) {
        animController->pause();
        if (pauseBtn) pauseBtn->setText("\u25B6");  // Play icon
    } else {
        animController->resume();
        if (pauseBtn) pauseBtn->setText("\u23F8");  // Pause icon
    }
}

void MainWindow::onStepForward()
{
    // Pause and step forward
    animController->pause();
    animController->stepForward();
}

void MainWindow::onStepBackward()
{
    // Pause and step backward
    animController->pause();
    animController->stepBackward();
    
    // Update UI state
    stepBackBtn->setEnabled(animController->canStepBackward());
}

void MainWindow::onTimelineSeek(int stepIndex)
{
    // Seek to specific step
    animController->pause();
    animController->seekToStep(stepIndex);
}

void MainWindow::onStepIndexChanged(int currentIndex, int totalSteps)
{
    // Update timeline
    timelineScrubber->setCurrentStep(currentIndex);
    
    // Update step counter
    stepCountLabel->setText(QString("Step: %1/%2").arg(currentIndex + 1).arg(totalSteps));
    
    // Enable/disable step buttons
    stepBackBtn->setEnabled(currentIndex > 0);
    stepFwdBtn->setEnabled(currentIndex < totalSteps - 1);
}

void MainWindow::onZoomIn()
{
    visualizer->zoomIn();
}

void MainWindow::onZoomOut()
{
    visualizer->zoomOut();
}

void MainWindow::onZoomReset()
{
    visualizer->resetZoom();
}

void MainWindow::onZoomChanged(qreal factor)
{
    int percentage = qRound(factor * 100);
    zoomLabel->setText(QString("%1%").arg(percentage));
}

void MainWindow::onPhaseChanged(const QString& phase)
{
    currentPhase = phase;
    updatePhaseIndicator(phase);
    
    // Show/hide heap widget based on phase (with splitter management)
    if (phase.contains("Merge", Qt::CaseInsensitive)) {
        heapWidget->show();
        if (mainSplitter && mainSplitter->count() > 1) {
            mainSplitter->widget(1)->show();
            // Animate splitter resize
            mainSplitter->setSizes({750, 220});
        }
    } else {
        if (mainSplitter && mainSplitter->count() > 1) {
            mainSplitter->widget(1)->hide();
            heapWidget->hide();
        }
    }
}

void MainWindow::updatePhaseIndicator(const QString& phase)
{
    // Compact label text (no "Phase:" prefix)
    phaseLabel->setText(phase);
    
    // Update color based on phase
    QString color = "#00b4d8";  // Default cyan
    if (phase.contains("Run", Qt::CaseInsensitive) || phase.contains("Generation", Qt::CaseInsensitive)) {
        color = "#4caf50";  // Green
    } else if (phase.contains("Merge", Qt::CaseInsensitive)) {
        color = "#ff9800";  // Orange
    } else if (phase.contains("Complete", Qt::CaseInsensitive) || phase.contains("Finished", Qt::CaseInsensitive)) {
        color = "#8bc34a";  // Light green
    } else if (phase.contains("Sort", Qt::CaseInsensitive)) {
        color = "#9c27b0";  // Purple
    }
    
    phaseLabel->setStyleSheet(QString(
        "QLabel {"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  padding: 4px 12px;"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #3d4451, stop:1 #2d3441);"
        "  border-radius: 3px;"
        "  color: %1;"
        "}"
    ).arg(color));
}

void MainWindow::onSpeedChanged(int value)
{
    // Map slider 1-10 to speed multiplier 0.25x - 4x
    double speed = 0.25 * qPow(2.0, (value - 1) / 3.0);
    animController->setSpeedMultiplier(speed);
    
    if (speedLabel) {
        speedLabel->setText(QString("%1x").arg(speed, 0, 'f', 1));
    }
    // Also update old UI if present
    if (ui->speedValueLabel) {
        ui->speedValueLabel->setText(QString("%1x").arg(speed, 0, 'f', 1));
    }
}

void MainWindow::onQueueSizeChanged(int size)
{
    if (ui->queueLabel) {
        ui->queueLabel->setText(QString("Queue: %1").arg(size));
    }
}
