#pragma once

#include <QObject>
#include <QQueue>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "sort_step.h"

class DataBlock;

class AnimationManager : public QObject
{
    Q_OBJECT

public:
    explicit AnimationManager(QGraphicsScene* scene);
    
    // Playback controls
    void setPaused(bool paused);
    void stepForward();
    void setSpeed(double speed);
    double getSpeed() const { return playbackSpeed; }
    bool isPaused() const { return paused; }
    int queueSize() const { return queue.size(); }
    
    // Clear all
    void clear();

public slots:
    void enqueueStep(const SortStep& step);
    void displayStep(StepType type, const QVector<double>& data, int progress);

signals:
    void queueSizeChanged(int size);
    void stepPlayed(const SortStep& step);

private slots:
    void playNext();
    void onAnimationFinished();

private:
    void playStep(const SortStep& step);
    void animateBars(const SortStep& step);
    void drawDiskRAMVisualization(bool inRAM);
    void drawMergeArrows(int leftIdx, int rightIdx, int targetIdx);
    void highlightCompare(int idx1, int idx2);
    void animateWrite(int idx);
    void colorRuns(const QVector<double>& data);
    QColor getColorForType(StepType type);
    QColor getRunColor(int runIndex);

    QQueue<SortStep> queue;
    QGraphicsScene* scene;
    QGraphicsTextItem* statusText;
    QGraphicsRectItem* ramBox;
    QGraphicsRectItem* diskBox;
    QGraphicsTextItem* ramLabel;
    QGraphicsTextItem* diskLabel;
    QList<DataBlock*> currentBlocks;
    QList<QGraphicsLineItem*> arrows;
    
    QTimer* playTimer;
    QParallelAnimationGroup* animGroup;
    
    bool playing;
    bool paused;
    double playbackSpeed;
    int baseDelay;
};
