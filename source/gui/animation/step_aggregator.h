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

/** @brief A raw algorithm operation emitted by the sorting algorithm. */
struct RawOperation {
    /** @brief Operation types produced by the sorting algorithm. */
    enum class Type {
        ReadBlock,     ///< Read a block from disk.
        WriteBlock,    ///< Write a block to disk.
        LoadToRAM,     ///< Load a block into RAM.
        UnloadFromRAM, ///< Remove a block from RAM.
        Compare,       ///< Compare two values.
        Swap,          ///< Swap two values.
        CreateRun,     ///< Create a new sorted run file.
        MergeRead,     ///< Read from a run during merge.
        MergeWrite,    ///< Write to output during merge.
        PhaseChange,   ///< Algorithm phase change.
        Complete       ///< Sorting is complete.
    };
    
    Type    type        = Type::ReadBlock; ///< The type of raw operation.
    int     blockIndex  = -1;              ///< Primary block index.
    int     secondIndex = -1;              ///< Secondary index (for swap/compare).
    double  value       = 0.0;             ///< Data value associated with this operation.
    QString metadata;                      ///< Phase name or other metadata.
    qint64  timestamp   = 0;               ///< Timestamp for operation ordering.
};

/** @brief Configuration controlling how raw operations are grouped into visual steps. */
struct AggregationConfig {
    int maxOpsPerLoadStep  = 50;  ///< Maximum blocks batched into one LoadChunk step.
    int maxOpsPerSortStep  = 100; ///< Maximum comparisons batched into one Sort step.
    int maxOpsPerMergeStep = 20;  ///< Maximum merge operations batched into one MergeStep.

    double loadDuration  = 300;  ///< Base display duration (ms) for a LoadChunk step.
    double sortDuration  = 400;  ///< Base display duration (ms) for a Sort step.
    double writeDuration = 250;  ///< Base display duration (ms) for a WriteRun step.
    double mergeDuration = 350;  ///< Base display duration (ms) for a MergeStep.
    double phaseDuration = 1500; ///< Base display duration (ms) for a PhaseTransition step.

    double introMultiplier  = 2.0; ///< Pacing multiplier applied during the intro phase (slower).
    double normalMultiplier = 1.0; ///< Pacing multiplier during normal operation.
    double fastMultiplier   = 0.6; ///< Pacing multiplier during later steps (faster).

    int introPaceUntilStep  = 20;  ///< Step count threshold where intro pacing ends.
    int normalPaceUntilStep = 100; ///< Step count threshold where normal pacing ends.
};

class StepAggregator : public QObject
{
    Q_OBJECT

public:
    explicit StepAggregator(QObject* parent = nullptr);
    ~StepAggregator() = default;
    
    /// @brief Sets the aggregation configuration.
    void setConfig(const AggregationConfig& config) { m_config = config; }
    /// @brief Returns the current aggregation configuration.
    const AggregationConfig& config() const { return m_config; }

    /// @brief Enqueues a single raw algorithm operation for aggregation.
    void addOperation(const RawOperation& op);
    /// @brief Enqueues a batch of raw algorithm operations for aggregation.
    void addOperations(const QVector<RawOperation>& ops);

    /// @brief Processes all pending raw operations into aggregated steps.
    void process();
    /// @brief Clears all pending raw operations and aggregated output.
    void clear();

    bool                    hasSteps()   const { return !m_outputSteps.isEmpty(); } ///< True if aggregated steps are available.
    /// @brief Removes and returns the next aggregated step.
    AggregatedStep          takeNextStep();
    /// @brief Removes and returns all aggregated steps.
    QVector<AggregatedStep> takeAllSteps();
    int                     stepCount()  const { return m_outputSteps.size(); } ///< Number of aggregated steps queued.

    int    totalRawOps()          const { return m_totalRawOps; }  ///< Total raw operations ever enqueued.
    int    totalAggregatedSteps() const { return m_totalAggSteps; } ///< Total aggregated steps ever produced.
    /// @brief Returns the ratio of raw operations to aggregated steps.
    double aggregationRatio() const { 
        return m_totalAggSteps > 0 ? static_cast<double>(m_totalRawOps) / m_totalAggSteps : 0; 
    }

signals:
    void stepsReady(int count);
    void processingComplete();

private:
    void aggregateLoadPhase(int startIdx, int& endIdx);
    void aggregateSortPhase(int startIdx, int& endIdx);
    void aggregateWritePhase(int startIdx, int& endIdx);
    void aggregateMergePhase(int startIdx, int& endIdx);
    void aggregatePhaseTransition(int startIdx, int& endIdx);

    AggregatedStep createStep(AggregatedStep::Type type, 
                               const QVector<int>& sources,
                               const QVector<double>& values,
                               const QString& status,
                               double baseDuration);
    
    /// @brief Returns the timing multiplier for the current step count.
    double getTimingMultiplier() const;

    AggregationConfig m_config; ///< Active aggregation configuration.

    QVector<RawOperation> m_rawOps;          ///< Buffer of pending raw operations.
    int                   m_processedIdx = 0; ///< Index up to which raw ops have been processed.

    QQueue<AggregatedStep> m_outputSteps; ///< Queue of produced aggregated steps.

    int m_totalRawOps   = 0; ///< Total raw operations ever enqueued.
    int m_totalAggSteps = 0; ///< Total aggregated steps ever produced.

    QString m_currentPhase; ///< Current algorithm phase label.
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
