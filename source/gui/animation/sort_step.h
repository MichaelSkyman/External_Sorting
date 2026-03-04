#pragma once
#include <QVector>

/** @brief Basic operation type for a legacy sort step. */
enum class StepType {
    READ,   ///< Block read from disk.
    SORT,   ///< Block sorted in RAM.
    WRITE,  ///< Block written to disk.
    MERGE   ///< Blocks merged.
};

/** @brief A single step in the legacy (QGraphicsScene) sort animation pipeline. */
struct SortStep {
    StepType     type; ///< The operation performed in this step.
    QVector<int> data; ///< Data values associated with this step.
};