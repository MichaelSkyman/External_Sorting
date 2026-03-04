#pragma once

#include <QPointF>
#include <QVector>
#include <QString>
#include <QVariant>

/** @brief Step types for the animation pipeline. */
enum class StepType {
    IntroDisk,         ///< Show disk storage visualization during intro.
    HighlightRAM,      ///< Highlight the RAM buffer area.
    ShowArrow,         ///< Animate an arrow from disk to RAM.
    LoadToRAM,         ///< Load a block from disk into RAM.
    WriteToRun,        ///< Write a sorted block to a run file on disk.
    WriteOutput,       ///< Write a final merged element to the output file.
    SortBlock,         ///< Sort a block in RAM.
    CompareElements,   ///< Highlight a comparison between two elements.
    SwapElements,      ///< Animate a swap of two elements.
    MergeStart,        ///< Begin the merge phase.
    MergeStep,         ///< Single k-way merge comparison step.
    MergeComplete,     ///< All runs have been fully merged.
    Pause,             ///< Pause playback for user comprehension.
    CameraFocus,       ///< Shift the camera/viewport focus point.
    ZoomRAM,           ///< Apply a zoom effect on the RAM area.
    StatusUpdate,      ///< Update the status text in the UI.
    PhaseTransition    ///< Major phase transition (e.g., Sort → Merge).
};

/** @brief Represents a single step in the animation pipeline. */
struct AnimationStep {
    StepType type = StepType::Pause; ///< The operation this step performs.

    QVector<double> values;  ///< Data values involved in this step.
    QVector<int>    indices; ///< Element indices involved (e.g., for highlights/comparisons).

    QPointF sourcePos; ///< Starting position for movement animations.
    QPointF targetPos; ///< Target position for movement animations.

    int blockIndex = -1; ///< Index of the primary block affected (-1 = none).
    int runIndex   = -1; ///< Index of the run file (-1 = none).
    int progress   = 0;  ///< Overall progress percentage (0–100).

    int durationMs   = 500; ///< Duration of this step in milliseconds.
    int delayAfterMs = 0;   ///< Extra delay appended after the step completes.

    QString statusText; ///< Primary status message shown in the UI.
    QString detailText; ///< Additional detail text (optional).

    bool inRAM       = false; ///< True if the data is currently in RAM.
    bool highlighted = false; ///< True if this step should render highlighted.
    bool isAutoPause = false; ///< True if this pause was auto-inserted by the controller.

    QVariant customData; ///< Step-type-specific auxiliary data.
    
    /// @brief Factory methods for common animation steps
    /// @{
    static AnimationStep createIntroDisk(const QVector<double>& data) {
        AnimationStep step;
        step.type = StepType::IntroDisk;
        step.values = data;
        step.durationMs = 1500;
        step.statusText = "Disk contains unsorted data";
        return step;
    }
    
    static AnimationStep createHighlightRAM() {
        AnimationStep step;
        step.type = StepType::HighlightRAM;
        step.durationMs = 800;
        step.statusText = "RAM buffer ready";
        return step;
    }
    
    static AnimationStep createLoadToRAM(int blockIdx, const QVector<double>& data) {
        AnimationStep step;
        step.type = StepType::LoadToRAM;
        step.blockIndex = blockIdx;
        step.values = data;
        step.durationMs = 1000;
        step.inRAM = true;
        step.statusText = QString("Loading block %1 into RAM").arg(blockIdx + 1);
        return step;
    }
    
    static AnimationStep createSortBlock(int blockIdx, const QVector<double>& sortedData) {
        AnimationStep step;
        step.type = StepType::SortBlock;
        step.blockIndex = blockIdx;
        step.values = sortedData;
        step.durationMs = 1500;
        step.inRAM = true;
        step.statusText = QString("Sorting block %1 in RAM").arg(blockIdx + 1);
        return step;
    }
    
    static AnimationStep createWriteRun(int runIdx, const QVector<double>& data) {
        AnimationStep step;
        step.type = StepType::WriteToRun;
        step.runIndex = runIdx;
        step.values = data;
        step.durationMs = 800;
        step.statusText = QString("Writing run %1 to disk").arg(runIdx + 1);
        return step;
    }
    
    static AnimationStep createMergeStart(int numRuns) {
        AnimationStep step;
        step.type = StepType::MergeStart;
        step.blockIndex = numRuns;
        step.durationMs = 1200;
        step.statusText = QString("Starting merge of %1 runs").arg(numRuns);
        return step;
    }
    
    static AnimationStep createMergeStep(int idx1, int idx2, double selected) {
        AnimationStep step;
        step.type = StepType::MergeStep;
        step.indices = {idx1, idx2};
        step.values = {selected};
        step.durationMs = 400;
        step.highlighted = true;
        return step;
    }
    
    static AnimationStep createWriteOutput(double value, int position) {
        AnimationStep step;
        step.type = StepType::WriteOutput;
        step.values = {value};
        step.blockIndex = position;
        step.durationMs = 300;
        step.statusText = "Writing to output";
        return step;
    }
    
    static AnimationStep createPause(int durationMs = 1500, const QString& reason = "") {
        AnimationStep step;
        step.type = StepType::Pause;
        step.durationMs = durationMs;
        step.isAutoPause = true;
        step.statusText = reason;
        return step;
    }
    
    static AnimationStep createPhaseTransition(const QString& phaseName) {
        AnimationStep step;
        step.type = StepType::PhaseTransition;
        step.durationMs = 2000;
        step.statusText = phaseName;
        return step;
    }
    
    static AnimationStep createCameraFocus(QPointF target) {
        AnimationStep step;
        step.type = StepType::CameraFocus;
        step.targetPos = target;
        step.durationMs = 800;
        return step;
    }
    
    static AnimationStep createZoomRAM(float scale = 1.0f) {
        AnimationStep step;
        step.type = StepType::ZoomRAM;
        step.customData = scale;
        step.durationMs = 600;
        return step;
    }
    
    static AnimationStep createCompare(int idx1, int idx2) {
        AnimationStep step;
        step.type = StepType::CompareElements;
        step.indices = {idx1, idx2};
        step.durationMs = 300;
        step.highlighted = true;
        return step;
    }
    
    static AnimationStep createSwap(int idx1, int idx2) {
        AnimationStep step;
        step.type = StepType::SwapElements;
        step.indices = {idx1, idx2};
        step.durationMs = 400;
        return step;
    }
    /// @}
};
