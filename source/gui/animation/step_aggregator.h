#ifndef STEP_AGGREGATOR_H
#define STEP_AGGREGATOR_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include "external_sort_canvas.h"

/**
 * @brief StepAggregator - Merges multiple algorithm operations into visual steps
 * 
 * External sorting produces massive numbers of atomic operations.
 * Rendering every single operation would cause severe lag.
 * 
 * This aggregator:
 * 1. Collects raw algorithm operations
 * 2. Groups related operations (e.g., loading N blocks to RAM)
 * 3. Produces fewer, higher-level visual animation steps
 * 4. Maintains visual fidelity while drastically reducing render load
 * 
 * Example:
 *   100 individual block loads → 1 "LoadChunk" animation step
 *   50 swap operations → 1 "SortInRAM" animation step
 */

// Raw algorithm operation (emitted by sorting thread)
struct RawOperation {
    enum class Type {
        ReadBlock,          // Read block from disk
        WriteBlock,         // Write block to disk
        LoadToRAM,          // Load block into RAM
        UnloadFromRAM,      // Remove block from RAM
        Compare,            // Compare two values
        Swap,               // Swap two values
        CreateRun,          // Create new sorted run
        MergeRead,          // Read from run during merge
        MergeWrite,         // Write to output during merge
        PhaseChange,        // Algorithm phase change
        Complete            // Sorting complete
    };
    
    Type type = Type::ReadBlock;
    int blockIndex = -1;        // Primary block index
    int secondIndex = -1;       // Secondary index (for swap/compare)
    double value = 0.0;
    QString metadata;           // Phase name, etc.
    qint64 timestamp = 0;       // For ordering
};

// Configuration for aggregation behavior
struct AggregationConfig {
    // How many raw operations to batch into one visual step
    int maxOpsPerLoadStep = 50;     // Max blocks in one "LoadChunk" step
    int maxOpsPerSortStep = 100;    // Max comparisons in one "Sort" step
    int maxOpsPerMergeStep = 20;    // Max merge operations per step
    
    // Animation timing (base durations in ms)
    double loadDuration = 300;
    double sortDuration = 400;
    double writeDuration = 250;
    double mergeDuration = 350;
    double phaseDuration = 1500;
    
    // Educational pacing multipliers
    double introMultiplier = 2.0;   // Slower at start
    double normalMultiplier = 1.0;
    double fastMultiplier = 0.6;    // Faster later
    
    // At what step counts to change pace
    int introPaceUntilStep = 20;
    int normalPaceUntilStep = 100;
};

class StepAggregator : public QObject
{
    Q_OBJECT

public:
    explicit StepAggregator(QObject* parent = nullptr);
    ~StepAggregator() = default;
    
    // Configuration
    void setConfig(const AggregationConfig& config) { m_config = config; }
    const AggregationConfig& config() const { return m_config; }
    
    // Input: raw algorithm operations
    void addOperation(const RawOperation& op);
    void addOperations(const QVector<RawOperation>& ops);
    
    // Processing
    void process();
    void clear();
    
    // Output: aggregated visual steps
    bool hasSteps() const { return !m_outputSteps.isEmpty(); }
    AggregatedStep takeNextStep();
    QVector<AggregatedStep> takeAllSteps();
    int stepCount() const { return m_outputSteps.size(); }
    
    // Statistics
    int totalRawOps() const { return m_totalRawOps; }
    int totalAggregatedSteps() const { return m_totalAggSteps; }
    double aggregationRatio() const { 
        return m_totalAggSteps > 0 ? static_cast<double>(m_totalRawOps) / m_totalAggSteps : 0; 
    }

signals:
    void stepsReady(int count);
    void processingComplete();

private:
    // Aggregation methods for each phase
    void aggregateLoadPhase(int startIdx, int& endIdx);
    void aggregateSortPhase(int startIdx, int& endIdx);
    void aggregateWritePhase(int startIdx, int& endIdx);
    void aggregateMergePhase(int startIdx, int& endIdx);
    void aggregatePhaseTransition(int startIdx, int& endIdx);
    
    // Helper to create aggregated step with timing
    AggregatedStep createStep(AggregatedStep::Type type, 
                               const QVector<int>& sources,
                               const QVector<double>& values,
                               const QString& status,
                               double baseDuration);
    
    // Get timing multiplier based on step count
    double getTimingMultiplier() const;
    
    AggregationConfig m_config;
    
    // Input buffer
    QVector<RawOperation> m_rawOps;
    int m_processedIdx = 0;
    
    // Output buffer
    QQueue<AggregatedStep> m_outputSteps;
    
    // Statistics
    int m_totalRawOps = 0;
    int m_totalAggSteps = 0;
    
    // Current phase tracking
    QString m_currentPhase;
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline StepAggregator::StepAggregator(QObject* parent)
    : QObject(parent)
{
}

inline void StepAggregator::addOperation(const RawOperation& op)
{
    m_rawOps.append(op);
    m_totalRawOps++;
}

inline void StepAggregator::addOperations(const QVector<RawOperation>& ops)
{
    m_rawOps.append(ops);
    m_totalRawOps += ops.size();
}

inline void StepAggregator::process()
{
    while (m_processedIdx < m_rawOps.size()) {
        const RawOperation& op = m_rawOps[m_processedIdx];
        int nextIdx = m_processedIdx;
        
        switch (op.type) {
            case RawOperation::Type::PhaseChange:
                aggregatePhaseTransition(m_processedIdx, nextIdx);
                break;
                
            case RawOperation::Type::LoadToRAM:
            case RawOperation::Type::ReadBlock:
                aggregateLoadPhase(m_processedIdx, nextIdx);
                break;
                
            case RawOperation::Type::Compare:
            case RawOperation::Type::Swap:
                aggregateSortPhase(m_processedIdx, nextIdx);
                break;
                
            case RawOperation::Type::CreateRun:
            case RawOperation::Type::WriteBlock:
                aggregateWritePhase(m_processedIdx, nextIdx);
                break;
                
            case RawOperation::Type::MergeRead:
            case RawOperation::Type::MergeWrite:
                aggregateMergePhase(m_processedIdx, nextIdx);
                break;
                
            case RawOperation::Type::Complete:
                {
                    AggregatedStep step;
                    step.type = AggregatedStep::Type::Complete;
                    step.statusText = "Sorting Complete!";
                    step.duration = 2000;
                    m_outputSteps.enqueue(step);
                    m_totalAggSteps++;
                    nextIdx = m_processedIdx + 1;
                }
                break;
                
            default:
                nextIdx = m_processedIdx + 1;
                break;
        }
        
        m_processedIdx = nextIdx;
    }
    
    if (!m_outputSteps.isEmpty()) {
        emit stepsReady(m_outputSteps.size());
    }
    emit processingComplete();
}

inline void StepAggregator::clear()
{
    m_rawOps.clear();
    m_outputSteps.clear();
    m_processedIdx = 0;
    m_totalRawOps = 0;
    m_totalAggSteps = 0;
}

inline AggregatedStep StepAggregator::takeNextStep()
{
    return m_outputSteps.isEmpty() ? AggregatedStep() : m_outputSteps.dequeue();
}

inline QVector<AggregatedStep> StepAggregator::takeAllSteps()
{
    QVector<AggregatedStep> result;
    while (!m_outputSteps.isEmpty()) {
        result.append(m_outputSteps.dequeue());
    }
    return result;
}

inline void StepAggregator::aggregateLoadPhase(int startIdx, int& endIdx)
{
    QVector<int> blockIndices;
    QVector<double> values;
    
    int i = startIdx;
    while (i < m_rawOps.size() && blockIndices.size() < m_config.maxOpsPerLoadStep) {
        const RawOperation& op = m_rawOps[i];
        
        if (op.type == RawOperation::Type::LoadToRAM || 
            op.type == RawOperation::Type::ReadBlock) {
            blockIndices.append(op.blockIndex);
            values.append(op.value);
            i++;
        } else {
            break;
        }
    }
    
    if (!blockIndices.isEmpty()) {
        AggregatedStep step = createStep(
            AggregatedStep::Type::LoadChunk,
            blockIndices,
            values,
            QString("Loading %1 blocks to RAM").arg(blockIndices.size()),
            m_config.loadDuration
        );
        step.operationCount = blockIndices.size();
        m_outputSteps.enqueue(step);
        m_totalAggSteps++;
    }
    
    endIdx = i;
}

inline void StepAggregator::aggregateSortPhase(int startIdx, int& endIdx)
{
    int compareCount = 0;
    int swapCount = 0;
    QVector<int> affectedIndices;
    
    int i = startIdx;
    while (i < m_rawOps.size() && (compareCount + swapCount) < m_config.maxOpsPerSortStep) {
        const RawOperation& op = m_rawOps[i];
        
        if (op.type == RawOperation::Type::Compare) {
            compareCount++;
            if (!affectedIndices.contains(op.blockIndex)) {
                affectedIndices.append(op.blockIndex);
            }
            if (!affectedIndices.contains(op.secondIndex)) {
                affectedIndices.append(op.secondIndex);
            }
            i++;
        } else if (op.type == RawOperation::Type::Swap) {
            swapCount++;
            if (!affectedIndices.contains(op.blockIndex)) {
                affectedIndices.append(op.blockIndex);
            }
            if (!affectedIndices.contains(op.secondIndex)) {
                affectedIndices.append(op.secondIndex);
            }
            i++;
        } else {
            break;
        }
    }
    
    if (compareCount > 0 || swapCount > 0) {
        AggregatedStep step = createStep(
            AggregatedStep::Type::SortInRAM,
            affectedIndices,
            {},
            QString("Sorting: %1 comparisons, %2 swaps").arg(compareCount).arg(swapCount),
            m_config.sortDuration
        );
        step.operationCount = compareCount + swapCount;
        m_outputSteps.enqueue(step);
        m_totalAggSteps++;
    }
    
    endIdx = i;
}

inline void StepAggregator::aggregateWritePhase(int startIdx, int& endIdx)
{
    QVector<int> blockIndices;
    QVector<double> values;
    
    int i = startIdx;
    while (i < m_rawOps.size() && blockIndices.size() < m_config.maxOpsPerLoadStep) {
        const RawOperation& op = m_rawOps[i];
        
        if (op.type == RawOperation::Type::CreateRun || 
            op.type == RawOperation::Type::WriteBlock) {
            blockIndices.append(op.blockIndex);
            values.append(op.value);
            i++;
        } else {
            break;
        }
    }
    
    if (!blockIndices.isEmpty()) {
        AggregatedStep step = createStep(
            AggregatedStep::Type::WriteRun,
            blockIndices,
            values,
            QString("Writing run with %1 blocks").arg(blockIndices.size()),
            m_config.writeDuration
        );
        step.operationCount = blockIndices.size();
        m_outputSteps.enqueue(step);
        m_totalAggSteps++;
    }
    
    endIdx = i;
}

inline void StepAggregator::aggregateMergePhase(int startIdx, int& endIdx)
{
    int mergeOps = 0;
    QVector<int> sourceBlocks;
    QVector<int> targetBlocks;
    QVector<double> values;
    
    int i = startIdx;
    while (i < m_rawOps.size() && mergeOps < m_config.maxOpsPerMergeStep) {
        const RawOperation& op = m_rawOps[i];
        
        if (op.type == RawOperation::Type::MergeRead) {
            sourceBlocks.append(op.blockIndex);
            mergeOps++;
            i++;
        } else if (op.type == RawOperation::Type::MergeWrite) {
            targetBlocks.append(op.blockIndex);
            values.append(op.value);
            mergeOps++;
            i++;
        } else {
            break;
        }
    }
    
    if (mergeOps > 0) {
        AggregatedStep step;
        step.type = AggregatedStep::Type::MergeStep;
        step.sourceBlocks = sourceBlocks;
        step.targetBlocks = targetBlocks;
        step.values = values;
        step.statusText = QString("Merging %1 elements").arg(mergeOps);
        step.duration = m_config.mergeDuration * getTimingMultiplier();
        step.operationCount = mergeOps;
        m_outputSteps.enqueue(step);
        m_totalAggSteps++;
    }
    
    endIdx = i;
}

inline void StepAggregator::aggregatePhaseTransition(int startIdx, int& endIdx)
{
    const RawOperation& op = m_rawOps[startIdx];
    
    m_currentPhase = op.metadata;
    
    AggregatedStep step;
    step.type = AggregatedStep::Type::PhaseTransition;
    step.statusText = op.metadata;
    step.duration = m_config.phaseDuration;
    step.operationCount = 1;
    
    m_outputSteps.enqueue(step);
    m_totalAggSteps++;
    
    endIdx = startIdx + 1;
}

inline AggregatedStep StepAggregator::createStep(
    AggregatedStep::Type type,
    const QVector<int>& sources,
    const QVector<double>& values,
    const QString& status,
    double baseDuration)
{
    AggregatedStep step;
    step.type = type;
    step.sourceBlocks = sources;
    step.values = values;
    step.statusText = status;
    step.duration = baseDuration * getTimingMultiplier();
    return step;
}

inline double StepAggregator::getTimingMultiplier() const
{
    if (m_totalAggSteps < m_config.introPaceUntilStep) {
        return m_config.introMultiplier;
    } else if (m_totalAggSteps < m_config.normalPaceUntilStep) {
        return m_config.normalMultiplier;
    } else {
        return m_config.fastMultiplier;
    }
}

#endif // STEP_AGGREGATOR_H
