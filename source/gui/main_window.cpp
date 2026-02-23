#include "main_window.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QButtonGroup>

#include "../base/external_sort.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    connect(browseInputBtn, &QPushButton::clicked,
            this, &MainWindow::browseInput);

    connect(browseOutputBtn, &QPushButton::clicked,
            this, &MainWindow::browseOutput);

    connect(startBtn, &QPushButton::clicked,
            this, &MainWindow::startSorting);

    connect(autoRadio, &QRadioButton::toggled,
            this, &MainWindow::onChunkModeChanged);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    setWindowTitle("External Sorting Demo");
    setMinimumSize(500, 400);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ========== File Selection Group ==========
    QGroupBox *fileGroup = new QGroupBox("File Selection", this);
    QFormLayout *fileLayout = new QFormLayout(fileGroup);
    fileLayout->setSpacing(10);

    // Input file row
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputEdit = new QLineEdit(this);
    inputEdit->setPlaceholderText("Select input file...");
    browseInputBtn = new QPushButton("Browse...", this);
    browseInputBtn->setFixedWidth(100);
    inputLayout->addWidget(inputEdit);
    inputLayout->addWidget(browseInputBtn);
    fileLayout->addRow("Input File:", inputLayout);

    // Output file row
    QHBoxLayout *outputLayout = new QHBoxLayout();
    outputEdit = new QLineEdit(this);
    outputEdit->setPlaceholderText("Select output file...");
    browseOutputBtn = new QPushButton("Browse...", this);
    browseOutputBtn->setFixedWidth(100);
    outputLayout->addWidget(outputEdit);
    outputLayout->addWidget(browseOutputBtn);
    fileLayout->addRow("Output File:", outputLayout);

    mainLayout->addWidget(fileGroup);

    // ========== Chunk Settings Group ==========
    QGroupBox *chunkGroup = new QGroupBox("Chunk Settings", this);
    QVBoxLayout *chunkLayout = new QVBoxLayout(chunkGroup);
    chunkLayout->setSpacing(10);

    autoRadio = new QRadioButton("Auto (based on available memory)", this);
    autoRadio->setChecked(true);
    chunkLayout->addWidget(autoRadio);

    QHBoxLayout *manualLayout = new QHBoxLayout();
    manualRadio = new QRadioButton("Manual:", this);
    chunkSpinBox = new QSpinBox(this);
    chunkSpinBox->setRange(1, 1000000);
    chunkSpinBox->setValue(10000);
    chunkSpinBox->setSuffix(" elements");
    chunkSpinBox->setEnabled(false);
    chunkLabel = new QLabel("Chunk size:", this);
    chunkLabel->setEnabled(false);
    manualLayout->addWidget(manualRadio);
    manualLayout->addWidget(chunkSpinBox);
    manualLayout->addStretch();
    chunkLayout->addLayout(manualLayout);

    QButtonGroup *modeGroup = new QButtonGroup(this);
    modeGroup->addButton(autoRadio);
    modeGroup->addButton(manualRadio);

    mainLayout->addWidget(chunkGroup);

    // ========== Progress Section ==========
    QGroupBox *progressGroup = new QGroupBox("Progress", this);
    QVBoxLayout *progressLayout = new QVBoxLayout(progressGroup);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressLayout->addWidget(progressBar);

    statusLabel = new QLabel("Ready", this);
    statusLabel->setStyleSheet("color: gray;");
    progressLayout->addWidget(statusLabel);

    mainLayout->addWidget(progressGroup);

    // ========== Start Button ==========
    startBtn = new QPushButton("Start Sorting", this);
    startBtn->setMinimumHeight(40);
    startBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3d8b40; }"
    );
    mainLayout->addWidget(startBtn);

    mainLayout->addStretch();
}

void MainWindow::onChunkModeChanged() {
    bool isManual = manualRadio->isChecked();
    chunkSpinBox->setEnabled(isManual);
}

void MainWindow::browseInput() {
    QString file = QFileDialog::getOpenFileName(
        this, "Select Input File", "", "Binary Files (*.bin);;All Files (*)");

    if (!file.isEmpty()) {
        inputEdit->setText(file);
        config.setInputFile(file.toStdString());
    }
}

void MainWindow::browseOutput() {
    QString file = QFileDialog::getSaveFileName(
        this, "Select Output File", "", "Binary Files (*.bin);;All Files (*)");

    if (!file.isEmpty()) {
        outputEdit->setText(file);
        config.setOutputFile(file.toStdString());
    }
}

void MainWindow::startSorting() {
    if (inputEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select an input file.");
        return;
    }
    if (outputEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select an output file.");
        return;
    }

    config.setInputFile(inputEdit->text().toStdString());
    config.setOutputFile(outputEdit->text().toStdString());

    if (autoRadio->isChecked()) {
        config.setChunkMode(ChunkMode::AUTO);
    } else {
        config.setChunkMode(ChunkMode::MANUAL);
        config.setManualChunkSize(chunkSpinBox->value());
    }

    size_t chunkSize = config.getChunkSize();

    statusLabel->setText("Sorting in progress...");
    statusLabel->setStyleSheet("color: blue;");
    progressBar->setValue(50);
    startBtn->setEnabled(false);

    try {
        ExternalSorter sorter(chunkSize);
        sorter.sort(config.getInputFile(), config.getOutputFile());

        progressBar->setValue(100);
        statusLabel->setText("Sorting completed successfully!");
        statusLabel->setStyleSheet("color: green;");
        QMessageBox::information(this, "Done", "Sorting completed!");

    } catch (const std::exception& e) {
        progressBar->setValue(0);
        statusLabel->setText("Sorting failed!");
        statusLabel->setStyleSheet("color: red;");
        QMessageBox::critical(this, "Error", 
            QString("Sorting failed: %1").arg(e.what()));
    } catch (...) {
        progressBar->setValue(0);
        statusLabel->setText("Sorting failed!");
        statusLabel->setStyleSheet("color: red;");
        QMessageBox::critical(this, "Error", "Sorting failed!");
    }

    startBtn->setEnabled(true);
}


