#pragma once

#include <QPointF>
#include <QVector>
#include <QString>
#include <QVariant>

// Step types for the animation pipeline
enum class StepType {
    // Introduction sequence
    IntroDisk,          // Show disk storage visualization
    HighlightRAM,       // Highlight RAM area
    ShowArrow,          // Animate arrow from disk to RAM
    
    // Data movement
    LoadToRAM,          // Load block from disk to RAM
    WriteToRun,         // Write sorted data to run file
    WriteOutput,        // Write final merged output
    
    // Sorting operations
    SortBlock,          // Sort block in RAM
    CompareElements,    // Highlight comparison
    SwapElements,       // Animate swap
    
    // Merge operations
    MergeStart,         // Begin merge phase
    MergeStep,          // Single merge comparison
    MergeComplete,      // Merge finished
    
    // Control
    Pause,              // Pause for user understanding
    CameraFocus,        // Move camera/viewport
    ZoomRAM,            // Zoom effect on RAM
    
    // Status updates
    StatusUpdate,       // Update status text
    PhaseTransition     // Major phase change
};

// Represents a single animation step
struct AnimationStep {
    StepType type = StepType::Pause;
    
    // Data for the step
    QVector<double> values;         // Data values involved
    QVector<int> indices;           // Indices involved (for highlights)
    
    // Positions for movement animations
    QPointF sourcePos;
    QPointF targetPos;
    
    // Additional parameters
    int blockIndex = -1;            // Which block is affected
    int runIndex = -1;              // Which run file
    int progress = 0;               // Progress percentage
    
    // Timing
    int durationMs = 500;           // How long this step takes
    int delayAfterMs = 0;           // Delay after completion
    
    // Text/status
    QString statusText;
    QString detailText;
    
    // Flags
    bool inRAM = false;             // Is data in RAM?
    bool highlighted = false;       // Should be highlighted?
    bool isAutoPause = false;       // Auto-inserted pause?
    
    // Custom data
    QVariant customData;
    
    // Factory methods for common steps
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
};
