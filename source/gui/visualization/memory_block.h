#pragma once

#include <QRectF>
#include <QColor>
#include <QLinearGradient>
#include <QPainter>
#include <QtMath>

/** @brief Visual state of a memory block, controlling its rendered color. */
enum class BlockState {
    Disk,       ///< On disk – rendered gray.
    Loading,    ///< Being loaded from disk – rendered yellow.
    InRAM,      ///< In RAM buffer – rendered blue.
    Sorting,    ///< Being sorted in RAM – rendered purple.
    Compared,   ///< Being compared – rendered orange.
    Selected,   ///< Selected as merge minimum – rendered red.
    Writing,    ///< Being written to disk – rendered green.
    Output,     ///< In final output – rendered teal.
    Highlighted ///< Special highlight state.
};

/** @brief Represents a single data block for rendering in the sort visualizer. */
class MemoryBlock {
public:
    MemoryBlock() = default;
    
    explicit MemoryBlock(double value, int index = 0)
        : value(value)
        , index(index)
        , state(BlockState::Disk)
    {}
    
    double     value = 0.0;              ///< Numeric data value this block represents.
    int        index = 0;                ///< Position index in the original dataset.
    BlockState state = BlockState::Disk; ///< Current visual state controlling color.

    QRectF rect;           ///< Bounding rectangle in scene/widget coordinates.
    qreal  opacity  = 1.0; ///< Opacity (0.0–1.0) for fade effects.
    qreal  scale    = 1.0; ///< Uniform scale factor (1.0 = normal size).
    qreal  rotation = 0.0; ///< Rotation angle in degrees.

    qreal glowIntensity       = 0.0;   ///< Current glow intensity (0–1).
    qreal targetGlowIntensity = 0.0;   ///< Target glow intensity for smooth interpolation.
    qreal pulsePhase          = 0.0;   ///< Phase angle driving the pulsing animation.
    bool  isAnimating         = false; ///< True while any animated property is transitioning.

    qreal      colorTransitionProgress = 1.0;           ///< Progress (0–1) of the color state transition.
    BlockState previousState           = BlockState::Disk; ///< Previous state for color interpolation.
    
    /// @brief Returns the base color for the current visual state.
    QColor baseColor() const {
        switch (state) {
            case BlockState::Disk:
                return QColor(100, 102, 112);      // Slightly warmer gray
            case BlockState::Loading:
                return QColor(240, 190, 60);       // Brighter yellow
            case BlockState::InRAM:
                return QColor(65, 140, 210);       // Brighter blue
            case BlockState::Sorting:
                return QColor(160, 80, 210);       // Brighter purple
            case BlockState::Compared:
                return QColor(245, 130, 55);       // Brighter orange
            case BlockState::Selected:
                return QColor(220, 70, 90);        // Brighter red
            case BlockState::Writing:
                return QColor(60, 190, 110);       // Brighter green
            case BlockState::Output:
                return QColor(0, 160, 170);        // Brighter teal
            case BlockState::Highlighted:
                return QColor(255, 230, 110);      // Brighter yellow
            default:
                return QColor(80, 82, 92);
        }
    }
    
    /// @brief Creates a vertical gradient based on the block's base color.
    QLinearGradient createGradient() const {
        QColor base = baseColor();
        QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
        
        // Enhanced gradient with smoother transitions
        gradient.setColorAt(0.0, base.lighter(135));
        gradient.setColorAt(0.2, base.lighter(115));
        gradient.setColorAt(0.5, base);
        gradient.setColorAt(0.8, base.darker(115));
        gradient.setColorAt(1.0, base.darker(130));
        
        return gradient;
    }
    
    /// @brief Renders this block using @p painter.
    /// @param painter   QPainter to draw into.
    /// @param maxValue  Maximum value in the dataset, used for proportional height.
    void render(QPainter* painter, double maxValue = 100.0) const {
        if (rect.isEmpty() || opacity <= 0.01) return;
        
        painter->save();
        painter->setOpacity(opacity);
        
        // Apply transforms with smooth scaling
        if (scale != 1.0 || rotation != 0.0) {
            QPointF center = rect.center();
            painter->translate(center);
            painter->rotate(rotation);
            painter->scale(scale, scale);
            painter->translate(-center);
        }
        
        // Smooth shadow
        if (opacity > 0.5 && rect.width() > 15) {
            QRectF shadowRect = rect.adjusted(2, 2, 2, 2);
            painter->setBrush(QColor(0, 0, 0, 35));
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(shadowRect, 5, 5);
        }
        
        // Main body with gradient
        painter->setBrush(createGradient());
        painter->setPen(QPen(baseColor().darker(140), 1));
        painter->drawRoundedRect(rect, 6, 6);
        
        // Smooth glow effect - IMPROVED
        if (glowIntensity > 0.02) {
            QColor glowColor = baseColor().lighter(160);
            
            // Multiple glow layers for soft effect
            for (int i = 3; i > 0; i--) {
                qreal layerIntensity = glowIntensity * (0.15 + 0.15 * (3 - i));
                glowColor.setAlphaF(layerIntensity);
                
                QRectF glowRect = rect.adjusted(-i * 2.5, -i * 2.5, i * 2.5, i * 2.5);
                painter->setPen(QPen(glowColor, 2.0));
                painter->setBrush(Qt::NoBrush);
                painter->drawRoundedRect(glowRect, 6 + i, 6 + i);
            }
            
            // Inner bright glow
            if (glowIntensity > 0.5) {
                QColor innerGlow = glowColor;
                innerGlow.setAlphaF(glowIntensity * 0.2);
                painter->setBrush(innerGlow);
                painter->setPen(Qt::NoPen);
                painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 4, 4);
            }
        }
        
        // Value text - only if block is large enough
        if (rect.width() > 25 && rect.height() > 20) {
            painter->setPen(Qt::white);
            QFont font = painter->font();
            
            // Scale font based on block size
            int fontSize = qBound(8, static_cast<int>(rect.width() / 3.5), 14);
            font.setPixelSize(fontSize);
            font.setBold(true);
            painter->setFont(font);
            
            QString text = QString::number(value, 'f', 0);
            painter->drawText(rect, Qt::AlignCenter, text);
        }
        
        // Height bar (proportional to value) - only if tall enough
        if (maxValue > 0 && rect.height() > 35) {
            qreal barHeight = (value / maxValue) * (rect.height() - 14);
            barHeight = qMax(4.0, barHeight);  // Minimum visibility
            QRectF barRect(rect.left() + 3, rect.bottom() - barHeight - 6,
                          rect.width() - 6, barHeight);
            
            QColor barColor = baseColor().lighter(75);
            barColor.setAlpha(90);
            painter->fillRect(barRect, barColor);
        }
        
        painter->restore();
    }
    
    /// @brief Advances all animated properties by @p deltaTime seconds.
    void updateAnimation(double deltaTime) {
        // Smooth glow interpolation
        if (isAnimating) {
            targetGlowIntensity = 0.8;
            pulsePhase += deltaTime * 3.5;  // Slightly slower pulse
            if (pulsePhase > 2.0 * M_PI) {
                pulsePhase -= 2.0 * M_PI;
            }
            
            // Smooth pulsing glow
            qreal targetGlow = 0.35 + 0.35 * qSin(pulsePhase);
            glowIntensity = glowIntensity + (targetGlow - glowIntensity) * deltaTime * 8.0;
        } else {
            targetGlowIntensity = 0.0;
            // Smooth decay
            glowIntensity = glowIntensity * (1.0 - deltaTime * 4.0);
            if (glowIntensity < 0.01) glowIntensity = 0.0;
        }
        
        // State transition tracking
        if (colorTransitionProgress < 1.0) {
            colorTransitionProgress = qMin(1.0, colorTransitionProgress + deltaTime * 5.0);
        }
    }
    
    /// @brief Transitions this block to @p newState, recording the previous state.
    void changeStateTo(BlockState newState) {
        if (state != newState) {
            previousState = state;
            state = newState;
            colorTransitionProgress = 0.0;
        }
    }
};
