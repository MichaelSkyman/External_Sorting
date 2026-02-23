#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QProgressBar>
#include <QLabel>
#include <QGroupBox>
#include "../app/config.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void browseInput();
    void browseOutput();
    void startSorting();
    void onChunkModeChanged();

private:
    void setupUi();

    // Input/Output section
    QLineEdit *inputEdit;
    QPushButton *browseInputBtn;
    QLineEdit *outputEdit;
    QPushButton *browseOutputBtn;

    // Chunk settings section
    QRadioButton *autoRadio;
    QRadioButton *manualRadio;
    QSpinBox *chunkSpinBox;
    QLabel *chunkLabel;

    // Action section
    QPushButton *startBtn;
    QProgressBar *progressBar;
    QLabel *statusLabel;

    AppConfig config;
};
