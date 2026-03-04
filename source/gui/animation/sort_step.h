#pragma once
#include <QVector>

enum class StepType {
    READ,
    SORT,
    WRITE,
    MERGE,
    COMPARE,
    SWAP
};

struct SortStep {
    StepType type;
    QVector<double> data;
    int progress;
    bool inRAM;              // true = data in RAM, false = on disk
    int highlightIdx1;       // index to highlight (for compare)
    int highlightIdx2;       // second index to highlight
    int mergeLeftIdx;        // merge arrow source left
    int mergeRightIdx;       // merge arrow source right
    int mergeTargetIdx;      // merge arrow target
    QVector<int> runBoundaries; // boundaries of sorted runs
    
    SortStep() : progress(0), inRAM(true), highlightIdx1(-1), highlightIdx2(-1),
                 mergeLeftIdx(-1), mergeRightIdx(-1), mergeTargetIdx(-1) {}
};
