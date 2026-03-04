#pragma once

#include <QObject>
#include <QQueue>
#include <QGraphicsScene>
#include "sort_step.h"

/**
 * @brief Drives the legacy QGraphicsScene-based animation pipeline.
 *
 * Manages a queue of SortStep operations and plays them sequentially
 * against a QGraphicsScene.
 */
class AnimationManager : public QObject
{
    Q_OBJECT

public:
    explicit AnimationManager(QGraphicsScene* scene);

public slots:
    void enqueueStep(const SortStep& step);

private:
    void playNext();
    void playRead(const SortStep& step);

    QQueue<SortStep> queue;
    QGraphicsScene* scene;
    bool playing;
};