#pragma once

#include <QPointF>
#include <QEasingCurve>
#include "memory_block.h"

// Handles smooth animated movement of blocks
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
    
    // The underlying block
    MemoryBlock block;
    
    // Movement animation
    QPointF startPos;
    QPointF endPos;
    QPointF currentPos;
    double progress = 1.0;      // 0-1 animation progress
    bool isMoving = false;
    
    // Animation parameters
    double duration = 500.0;    // milliseconds
    double elapsed = 0.0;
    QEasingCurve easingCurve{QEasingCurve::InOutCubic};
    
    // Optional trajectory control
    QPointF controlPoint;       // For curved paths
    bool useBezier = false;
    
    // Scale/opacity transitions
    double startScale = 1.0;
    double endScale = 1.0;
    double startOpacity = 1.0;
    double endOpacity = 1.0;
    
    // Start movement animation
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
    
    // Start curved movement (for merge arrows etc)
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
    
    // Start scale animation
    void scaleTo(double targetScale, double durationMs = 300.0) {
        startScale = block.scale;
        endScale = targetScale;
        duration = durationMs;
        elapsed = 0.0;
        progress = 0.0;
        isMoving = true;
    }
    
    // Start fade animation
    void fadeTo(double targetOpacity, double durationMs = 300.0) {
        startOpacity = block.opacity;
        endOpacity = targetOpacity;
        duration = durationMs;
        elapsed = 0.0;
        progress = 0.0;
        isMoving = true;
    }
    
    // Update animation (call each frame)
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
    
    // Check if animation is complete
    bool isComplete() const {
        return !isMoving || progress >= 1.0;
    }
    
    // Get remaining time
    double remainingTime() const {
        return qMax(0.0, duration - elapsed);
    }
    
    // Render the block
    void render(QPainter* painter, double maxValue = 100.0) const {
        block.render(painter, maxValue);
    }
};

// Manages a collection of moving blocks
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
    
    // Move all blocks by offset
    void offsetAll(QPointF offset) {
        for (auto& block : blocks) {
            block.block.rect.translate(offset);
            block.startPos += offset;
            block.endPos += offset;
            block.currentPos += offset;
        }
    }
    
    // Arrange in a row
    void arrangeHorizontally(QPointF start, double spacing, double blockWidth, double blockHeight) {
        double x = start.x();
        for (auto& block : blocks) {
            block.block.rect = QRectF(x, start.y(), blockWidth, blockHeight);
            x += blockWidth + spacing;
        }
    }
    
    // Arrange in a grid
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
