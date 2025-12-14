/*
  ==============================================================================

    LayerTypes.h
    Created: 11 Dec 2025
    Author:  DevTools Refactored Layer System
    
    Description:
    Core types, enums, and structs for the layer system.
    Separated from Layer.h for better modularity and faster compilation.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <optional>
#include <memory>

namespace zaplab
{

//==============================================================================
// Forward declarations
class Layer;
using LayerPtr = std::shared_ptr<Layer>;
using LayerWeakPtr = std::weak_ptr<Layer>;

//==============================================================================
/**
 * @brief Enum for layer types - used for serialization and identification
 */
enum class LayerType
{
    Shape,
    Text,
    Image,
    Group
};

/**
 * @brief Enum for shape types
 */
enum class ShapeType
{
    None,
    Rectangle,
    Ellipse,
    Arc,
    Polygon,
    Star,
    Triangle,
    Line,
    Arrow
};

/**
 * @brief Enum defining how a layer responds to slider value changes
 */
enum class SliderLayerRole
{
    None,           ///< Static layer, doesn't respond to value
    Thumb,          ///< Moves/rotates with value
    Track,          ///< Background track
    ValueTrack,     ///< Fills based on value
    ModForward,     ///< Modulation indicator forward
    ModBackward,    ///< Modulation indicator backward
    Overlay         ///< Always on top
};

/**
 * @brief Stroke alignment - where the stroke is drawn relative to the path
 */
enum class StrokeAlignment
{
    Center,   ///< Stroke is centered on the path (half inside, half outside)
    Inside,   ///< Stroke is entirely inside the path
    Outside   ///< Stroke is entirely outside the path
};

/**
 * @brief Fill mode for shapes
 */
enum class FillMode
{
    Solid,
    LinearGradient,
    RadialGradient,
    ConicGradient,
    None
};

/**
 * @brief Sizing reference mode - determines how relative sizes are calculated
 * 
 * This affects how properties like corner radius, stroke width, shadow radius,
 * etc. scale when the slider/knob is resized.
 */
enum class SizingMode
{
    MinDimension,   ///< Scale relative to the smaller of width/height (best for knobs)
    MaxDimension,   ///< Scale relative to the larger of width/height
    Width,          ///< Always scale relative to width (best for horizontal sliders)
    Height,         ///< Always scale relative to height (best for vertical sliders)
    Average         ///< Scale relative to average of width/height
};

//==============================================================================
// Sizing utility functions
//==============================================================================

/**
 * @brief Get the reference dimension for sizing calculations
 * @param bounds The current render bounds
 * @param mode The sizing mode to use
 * @return The reference dimension in pixels
 */
[[nodiscard]] inline float getReferenceDimension(const juce::Rectangle<float>& bounds, SizingMode mode)
{
    switch (mode)
    {
        case SizingMode::MinDimension:  return juce::jmin(bounds.getWidth(), bounds.getHeight());
        case SizingMode::MaxDimension:  return juce::jmax(bounds.getWidth(), bounds.getHeight());
        case SizingMode::Width:         return bounds.getWidth();
        case SizingMode::Height:        return bounds.getHeight();
        case SizingMode::Average:       return (bounds.getWidth() + bounds.getHeight()) * 0.5f;
        default:                        return juce::jmin(bounds.getWidth(), bounds.getHeight());
    }
}

/**
 * @brief Convert a relative value (0-1) to absolute pixels
 * @param relativeValue The normalized value (0-1 range, where 1.0 = reference dimension)
 * @param bounds The current render bounds
 * @param mode The sizing mode
 * @return The absolute pixel value
 */
[[nodiscard]] inline float relativeToPixels(float relativeValue, const juce::Rectangle<float>& bounds, SizingMode mode)
{
    return relativeValue * getReferenceDimension(bounds, mode);
}

/**
 * @brief Convert an absolute pixel value to relative (0-1)
 * @param pixelValue The pixel value
 * @param bounds The current render bounds
 * @param mode The sizing mode
 * @return The normalized value (0-1 range)
 */
[[nodiscard]] inline float pixelsToRelative(float pixelValue, const juce::Rectangle<float>& bounds, SizingMode mode)
{
    const float ref = getReferenceDimension(bounds, mode);
    return ref > 0.0f ? pixelValue / ref : 0.0f;
}

//==============================================================================
/**
 * @brief Transform properties for a layer
 * Handles position, rotation, scale in a clean struct
 */
struct Transform
{
    juce::Point<float> position { 0.5f, 0.5f };     ///< Normalized 0-1, center by default
    juce::Point<float> anchor { 0.5f, 0.5f };       ///< Normalized 0-1, center by default
    float rotation { 0.0f };                        ///< Radians
    juce::Point<float> scale { 1.0f, 1.0f };
    juce::Point<float> size { 1.0f, 1.0f };         ///< Normalized 0-1, full size by default
    
    // Convenience setters
    void setPosition(float x, float y) noexcept { position = { x, y }; }
    void setSize(float w, float h) noexcept { size = { w, h }; }
    void setRotation(float radians) noexcept { rotation = radians; }
    
    // Convenience getters
    [[nodiscard]] juce::Point<float> getPosition() const noexcept { return position; }
    [[nodiscard]] juce::Point<float> getSize() const noexcept { return size; }
    [[nodiscard]] float getRotation() const noexcept { return rotation; }
    
    [[nodiscard]] juce::AffineTransform toAffineTransform(const juce::Rectangle<float>& bounds) const noexcept
    {
        const float cx = bounds.getX() + bounds.getWidth() * anchor.x;
        const float cy = bounds.getY() + bounds.getHeight() * anchor.y;
        
        return juce::AffineTransform()
            .scaled(scale.x, scale.y, cx, cy)
            .rotated(rotation, cx, cy)
            .translated(position.x, position.y);
    }
    
    bool operator==(const Transform& other) const noexcept
    {
        return position == other.position 
            && anchor == other.anchor 
            && rotation == other.rotation 
            && scale == other.scale
            && size == other.size;
    }
    
    bool operator!=(const Transform& other) const noexcept { return !(*this == other); }
};

//==============================================================================
/**
 * @brief Shadow effect properties
 * 
 * Note: Size-dependent properties (offset, radius, spread) are stored as 
 * RELATIVE values (0-1 range where 1.0 = reference dimension).
 */
struct Shadow
{
    bool enabled { false };
    juce::Colour colour { juce::Colours::black.withAlpha(0.5f) };
    juce::Point<float> offset { 0.02f, 0.02f };   ///< Relative offset (0.02 = 2% of ref)
    float radius { 0.04f };                        ///< Relative blur radius
    float spread { 0.0f };                         ///< Relative spread
    
    [[nodiscard]] juce::Point<float> getOffsetPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        const float ref = getReferenceDimension(bounds, mode);
        return { offset.x * ref, offset.y * ref };
    }
    
    [[nodiscard]] float getRadiusPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        return relativeToPixels(radius, bounds, mode);
    }
    
    [[nodiscard]] float getSpreadPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        return relativeToPixels(spread, bounds, mode);
    }
    
    bool operator==(const Shadow& other) const noexcept
    {
        return enabled == other.enabled 
            && colour == other.colour 
            && offset == other.offset 
            && radius == other.radius 
            && spread == other.spread;
    }
};

//==============================================================================
/**
 * @brief Gradient definition for fills
 */
enum class GradientType
{
    Linear,
    Radial,
    Conic
};

struct Gradient
{
    bool enabled { false };
    GradientType type { GradientType::Linear };
    bool isRadial { false };  ///< Legacy - use type instead
    
    std::vector<std::pair<float, juce::Colour>> stops;  ///< Position (0-1), Colour
    juce::Point<float> start { 0.5f, 0.0f };            ///< Normalized coordinates (default: top center)
    juce::Point<float> end { 0.5f, 1.0f };              ///< Normalized coordinates (default: bottom center)
    float rotation { 0.0f };                             ///< For linear gradients
    
    // Convenience accessors for two-color gradients
    juce::Colour colour1 { juce::Colours::white };
    juce::Colour colour2 { juce::Colours::black };
    
    [[nodiscard]] juce::ColourGradient toColourGradient(const juce::Rectangle<float>& bounds) const
    {
        const auto startPoint = juce::Point<float>(
            bounds.getX() + bounds.getWidth() * start.x,
            bounds.getY() + bounds.getHeight() * start.y
        );
        const auto endPoint = juce::Point<float>(
            bounds.getX() + bounds.getWidth() * end.x,
            bounds.getY() + bounds.getHeight() * end.y
        );
        
        bool useRadial = (type == GradientType::Radial) || isRadial;
        
        juce::ColourGradient gradient(
            stops.empty() ? colour1 : stops.front().second,
            startPoint,
            stops.empty() ? colour2 : stops.back().second,
            endPoint,
            useRadial
        );
        
        for (const auto& [pos, colour] : stops)
        {
            gradient.addColour(pos, colour);
        }
        
        return gradient;
    }
    
    static Gradient createDefault()
    {
        Gradient g;
        g.stops = { { 0.0f, juce::Colours::white }, { 1.0f, juce::Colours::black } };
        return g;
    }
};

//==============================================================================
/**
 * @brief Mask properties for layer masking
 */
struct LayerMask
{
    bool enabled { false };
    bool inverted { false };
    bool antialiased { true };
    LayerWeakPtr maskLayer;
    juce::Image maskImage;
    
    void clear()
    {
        enabled = false;
        maskLayer.reset();
        maskImage = juce::Image();
    }
};

//==============================================================================
/**
 * @brief Animation state for smooth transitions
 */
struct AnimationState
{
    bool isAnimating { false };
    float progress { 0.0f };     ///< 0-1
    float duration { 0.3f };     ///< seconds
    
    enum class EaseType { Linear, EaseIn, EaseOut, EaseInOut, Spring };
    EaseType easeType { EaseType::EaseOut };
    
    [[nodiscard]] float getEasedProgress() const noexcept
    {
        switch (easeType)
        {
            case EaseType::Linear:    return progress;
            case EaseType::EaseIn:    return progress * progress;
            case EaseType::EaseOut:   return 1.0f - (1.0f - progress) * (1.0f - progress);
            case EaseType::EaseInOut: return progress < 0.5f 
                                           ? 2.0f * progress * progress 
                                           : 1.0f - std::pow(-2.0f * progress + 2.0f, 2.0f) / 2.0f;
            case EaseType::Spring:    
            {
                const float c4 = (2.0f * juce::MathConstants<float>::pi) / 3.0f;
                return progress == 0.0f ? 0.0f : progress == 1.0f ? 1.0f
                    : std::pow(2.0f, -10.0f * progress) * std::sin((progress * 10.0f - 0.75f) * c4) + 1.0f;
            }
        }
        return progress;
    }
};

//==============================================================================
/**
 * @brief Hover/interaction state for layers
 */
struct InteractionState
{
    bool isHovered { false };
    bool isPressed { false };
    bool isSelected { false };
    bool isToggled { false };
    
    // Hover effect modifiers
    float hoverScaleMultiplier { 1.0f };
    float hoverRotation { 0.0f };
    juce::Point<float> hoverOffset { 0.0f, 0.0f };
    juce::Colour hoverFillColour { juce::Colours::transparentWhite };
    juce::Colour hoverStrokeColour { juce::Colours::transparentWhite };
    juce::Colour pressedFillColour { juce::Colours::transparentWhite };
};

} // namespace zaplab
