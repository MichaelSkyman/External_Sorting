#pragma once

#include <QPointF>
#include <QEasingCurve>
#include "memory_block.h"

/** @brief Wraps a MemoryBlock with smooth position, scale, and opacity animation. */
class MovingBlock {
public:
    MovingBlock() = default;
    
    explicit MovingBlock(const MemoryBlock& block)
        : block(block)
        , startPos(block.rect.topLeft())
        , endPos(block.rect.topLeft())
        , progress(1.0)
        , isMoving(false)
    {}
    
    MemoryBlock block; ///< The underlying block data and visual state.

    QPointF startPos;         ///< Animation start position.
    QPointF endPos;           ///< Animation end position.
    QPointF currentPos;       ///< Current interpolated position.
    double  progress  = 1.0;  ///< Normalized animation progress (0–1).
    bool    isMoving  = false; ///< True while a movement animation is in progress.

    double       duration    = 500.0;                    ///< Total animation duration in milliseconds.
    double       elapsed     = 0.0;                      ///< Time elapsed in the current animation (ms).
    QEasingCurve easingCurve{QEasingCurve::InOutCubic};  ///< Easing curve applied to progress.

    QPointF controlPoint;         ///< Bezier control point for curved-path movement.
    bool    useBezier = false;    ///< When true, movement follows a quadratic Bezier curve.

    double startScale   = 1.0; ///< Scale at the start of the animation.
    double endScale     = 1.0; ///< Scale at the end of the animation.
    double startOpacity = 1.0; ///< Opacity at the start of the animation.
    double endOpacity   = 1.0; ///< Opacity at the end of the animation.
    
    /// @brief Starts a linear movement animation to @p target.
    /// @param target     Destination position.
    /// @param durationMs Animation duration in milliseconds.
    void moveTo(QPointF target, double durationMs = 500.0) {
        startPos = block.rect.topLeft();
        endPos = target;
        currentPos = startPos;
        duration = durationMs;
        elapsed = 0.0;
        progress = 0.0;
        isMoving = true;
        useBezier = false;
    }
    
    /// @brief Starts a quadratic Bezier movement animation.
    /// @param target     Destination position.
    /// @param control    Bezier control point controlling the arc.
    /// @param durationMs Animation duration in milliseconds.
    void moveAlongCurve(QPointF target, QPointF control, double durationMs = 600.0) {
        startPos = block.rect.topLeft();
        endPos = target;
        controlPoint = control;
        currentPos = startPos;
        duration = durationMs;
        elapsed = 0.0;
        progress = 0.0;
        isMoving = true;
        useBezier = true;
    }
    
    /// @brief Starts a scale animation towards @p targetScale.
    void scaleTo(double targetScale, double durationMs = 300.0) {
        startScale = block.scale;
        endScale = targetScale;
        duration = durationMs;
        elapsed = 0.0;
        progress = 0.0;
        isMoving = true;
    }
    
    /// @brief Starts an opacity animation towards @p targetOpacity.
    void fadeTo(double targetOpacity, double durationMs = 300.0) {
        startOpacity = block.opacity;
        endOpacity = targetOpacity;
        duration = durationMs;
        elapsed = 0.0;
        progress = 0.0;
        isMoving = true;
    }
    
    /// @brief Advances the animation by @p deltaTimeMs milliseconds.
    void update(double deltaTimeMs) {
        if (!isMoving) return;
        
        elapsed += deltaTimeMs;
        progress = qMin(1.0, elapsed / duration);
        
        // Apply easing
        double easedProgress = easingCurve.valueForProgress(progress);
        
        // Update position
        if (useBezier) {
            // Quadratic bezier curve
            double t = easedProgress;
            double oneMinusT = 1.0 - t;
            currentPos = oneMinusT * oneMinusT * startPos
                       + 2.0 * oneMinusT * t * controlPoint
                       + t * t * endPos;
        } else {
            // Linear interpolation with easing
            currentPos = startPos + (endPos - startPos) * easedProgress;
        }
        
        // Update block position
        block.rect.moveTopLeft(currentPos);
        
        // Update scale
        block.scale = startScale + (endScale - startScale) * easedProgress;
        
        // Update opacity
        block.opacity = startOpacity + (endOpacity - startOpacity) * easedProgress;
        
        // Check completion
        if (progress >= 1.0) {
            isMoving = false;
            block.rect.moveTopLeft(endPos);
            block.scale = endScale;
            block.opacity = endOpacity;
        }
    }
    
    /// @brief Returns true when the animation has finished.
    bool isComplete() const {
        return !isMoving || progress >= 1.0;
    }
    
    /// @brief Returns the remaining animation time in milliseconds.
    double remainingTime() const {
        return qMax(0.0, duration - elapsed);
    }
    
    /// @brief Renders the block at its current animated position.
    void render(QPainter* painter, double maxValue = 100.0) const {
        block.render(painter, maxValue);
    }
};

/** @brief Manages a collection of MovingBlock instances as a group. */
class MovingBlockGroup {
public:
    QVector<MovingBlock> blocks;
    
    void clear() {
        blocks.clear();
    }
    
    void addBlock(const MemoryBlock& block) {
        blocks.append(MovingBlock(block));
    }
    
    void updateAll(double deltaTimeMs) {
        for (auto& block : blocks) {
            block.update(deltaTimeMs);
        }
    }
    
    bool allComplete() const {
        for (const auto& block : blocks) {
            if (!block.isComplete()) return false;
        }
        return true;
    }
    
    void renderAll(QPainter* painter, double maxValue = 100.0) const {
        for (const auto& block : blocks) {
            block.render(painter, maxValue);
        }
    }
    
    /// @brief Translates all blocks and their animation endpoints by @p offset.
    void offsetAll(QPointF offset) {
        for (auto& block : blocks) {
            block.block.rect.translate(offset);
            block.startPos += offset;
            block.endPos += offset;
            block.currentPos += offset;
        }
    }
    
    /// @brief Arranges all blocks in a horizontal row starting at @p start.
    void arrangeHorizontally(QPointF start, double spacing, double blockWidth, double blockHeight) {
        double x = start.x();
        for (auto& block : blocks) {
            block.block.rect = QRectF(x, start.y(), blockWidth, blockHeight);
            x += blockWidth + spacing;
        }
    }
    
    /// @brief Arranges all blocks in a grid layout starting at @p start.
    void arrangeGrid(QPointF start, int columns, double spacing, double blockWidth, double blockHeight) {
        int col = 0;
        int row = 0;
        for (auto& block : blocks) {
            double x = start.x() + col * (blockWidth + spacing);
            double y = start.y() + row * (blockHeight + spacing);
            block.block.rect = QRectF(x, y, blockWidth, blockHeight);
            
            col++;
            if (col >= columns) {
                col = 0;
                row++;
            }
        }
    }
};
